#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/ioctl.h>
//#include <asm/phys.h>
#include <asm/io.h>
// #include<linux/proc_fs.h>
#include <linux/dma-mapping.h>

// For device tree
#include <linux/of.h>
#include <linux/of_reserved_mem.h>

#define DEVICE_NAME "cma_dma_pl"
#define CLASS_NAME "allocContinuousMemoryDMAPL"

// Определение команд IOCTL
#define MEM_ALLOC _IOW('m', 1, size_t)
#define MEM_FREE  _IO('m', 2)
#define MEM_GET_PHYS_ADDR _IOR('m', 3, unsigned long)
#define MEM_GET_SIZE_ALLOC _IOR('m', 4, size_t)

#define LOW_ALLOC_SIZE_CMA 0x2000000



static int majorNumber;
static struct class *memory_class;
static struct device *memory_device;

static unsigned char flag_alloc_dtb = 0x00;
static void *dma_buffer = NULL; // Указатель на DMA-буфер
static dma_addr_t dma_handle;   // Физический адрес DMA
static size_t allocated_size = 0;




static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "cma_dma_pl: Устройство открыто\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "cma_dma_pl: Устройство закрыто\n");
    return 0;
}


static int dev_mmap(struct file *filep, struct vm_area_struct *vma) {
    size_t size = 0;
    unsigned long phaddr = 0;
    pgprot_t prot;
    if (!dma_buffer) {
        printk(KERN_ERR "cma_dma_pl: Память не выделена\n");
        return -EINVAL;
    }

    if (vma->vm_pgoff == 0) {
        phaddr = dma_handle >> PAGE_SHIFT;
    } else {
        phaddr = vma->vm_pgoff << PAGE_SHIFT;
    }
    
    size = vma->vm_end - vma->vm_start;
    printk(KERN_INFO "cma_dma_pl: Размер выделенный через маппинг в байтах = %zu\n", size);
    prot = pgprot_noncached(vma->vm_page_prot);

    if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size, prot)) {
        printk(KERN_ERR "cma_dma_pl: Ошибка отображения памяти\n");
        return -EAGAIN;
    }

    printk(KERN_INFO "cma_dma_pl: Память отображена пользователю через маппинг адрес = 0x%lx\n", phaddr);
    return 0;
}




static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case MEM_ALLOC: {
            size_t size;
            if (copy_from_user(&size, (void __user *)arg, sizeof(size))) {
                return -EFAULT;
            }
            if (flag_alloc_dtb == 0x01) {
                printk(KERN_WARNING "cma_dma_pl: Память уже выделена через device tree\n", size);
                return -ENOMEM;
            }
            if (dma_buffer) {
                dma_free_coherent(memory_device, allocated_size, dma_buffer, dma_handle);
            }
            if ((size % PAGE_SIZE) != 0) {
                printk(KERN_WARNING "cma_dma_pl: Размер не выровнен по странице\n", size);
                return -ENOMEM;
            }
            dma_buffer = dma_alloc_coherent(memory_device, size, &dma_handle, GFP_KERNEL);
            if (!dma_buffer) {
                printk(KERN_ALERT "cma_dma_pl: Ошибка выделения DMA-памяти\n");
                return -ENOMEM;
            }
            allocated_size = size;
            printk(KERN_INFO "cma_dma_pl: Выделено %zu байт DMA-памяти\n", size);
            break;
        }
        case MEM_FREE: {
            if (flag_alloc_dtb == 0x01) {
                printk(KERN_WARNING "cma_dma_pl: Память уже выделена через device tree\n", size);
                return 0;
            }

            if (dma_buffer) {
                dma_free_coherent(memory_device, allocated_size, dma_buffer, dma_handle);
                dma_buffer = NULL;
                allocated_size = 0;
                printk(KERN_INFO "cma_dma_pl: CMA-память освобождена\n");
            } else {
                printk(KERN_WARNING "cma_dma_pl: Память не была выделена\n");
            }
            break;
        }
        case MEM_GET_PHYS_ADDR: {
            if (!dma_buffer) {
                printk(KERN_WARNING "cma_dma_pl: Память не выделена\n");
                return -EINVAL;
            }
            if (copy_to_user((void __user *)arg, &dma_handle, sizeof(dma_handle))) {
                return -EFAULT;
            }
            printk(KERN_INFO "cma_dma_pl: Возвращен физический адрес: 0x%lx\n", (unsigned long)dma_handle);
            break;
        }
        case MEM_GET_SIZE_ALLOC: {
            if (flag_alloc_dtb == 0x00) {
                allocated_size = 0;
                printk(KERN_ALERT "cma_dma_pl: Память не выделена через device tree\n");
                if (copy_to_user((void __user *)arg, &allocated_size, sizeof(allocated_size))) {
                    return -EFAULT;
                }
                return -ENOMEM;
            }
            if (!dma_buffer) {
                printk(KERN_ALERT "cma_dma_pl: Ошибка выделения DMA-памяти\n");
                return -ENOMEM;
            }
            if (copy_to_user((void __user *)arg, &allocated_size, sizeof(allocated_size))) {
                return -EFAULT;
            }
            printk(KERN_INFO "cma_dma_pl: Возвращен выделеный размер в байтах: %zu\n", allocated_size);
            break;
        }
        default:
            return -EINVAL;
    }
    return 0;
}


static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
    .mmap = dev_mmap
};



static int kmem_init(void) {

    unsigned int remains;

    printk(KERN_INFO "cma_dma_pl: Инициализация драйвера\n");

    // Регистрируем символьное устройство
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "cma_dma_pl: Ошибка регистрации устройства\n");
        return majorNumber;
    }
    printk(KERN_INFO "cma_dma_pl: Зарегистрировано устройство с major number %d\n", majorNumber);

    // Создаем класс устройства
    // memory_class = class_create(CLASS_NAME);
    memory_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(memory_class)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "cma_dma_pl: Ошибка создания класса устройства\n");
        return PTR_ERR(memory_class);
    }
    printk(KERN_INFO "cma_dma_pl: Класс устройства создан\n");

    // Создаем устройство
    memory_device = device_create(memory_class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(memory_device)) {
        class_destroy(memory_class);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "cma_dma_pl: Ошибка создания устройства\n");
        return PTR_ERR(memory_device);
    }
    printk(KERN_INFO "cma_dma_pl: Устройство создано\n");

    struct device_node *cma_node;
    unsigned int cma_align_addr;
    unsigned int cma_size;
    /* Найти узел с compatible = "shared-dma-pool" */
    cma_node = of_find_compatible_node(NULL, NULL, "shared-dma-pool");
    if (!cma_node) {
        printk(KERN_WARNING "cma_dma_pl: Ошибка нахождении узла в device tree\n");
        flag_alloc_dtb = 0x00;
    } else {
        flag_alloc_dtb = 0x01;
        /* Получить размер памяти */
        if (of_property_read_u32(cma_node, "size", &cma_size)) {
            printk(KERN_WARNING "cma_dma_pl: Ошибка нахождении резмера в device tree\n");
        }
        if (of_property_read_u32(cma_node, "alignment", &cma_align_addr)) {
            printk(KERN_WARNING "cma_dma_pl: Ошибка нахождении резмера в device tree\n");
            cma_align_addr = 0x1000;
        }
        allocated_size = (size_t)cma_size;
        remains = allocated_size % cma_align_addr; 
        if (remains != 0) {
            allocated_size -= remains; // Нужно выравнять по странице
        }
        allocated_size -= LOW_ALLOC_SIZE_CMA; // Весгда выделять на LOW_ALLOC_SIZE_CMA байт меньше для нормального функционирования ОС
        dma_buffer = dma_alloc_coherent(memory_device, allocated_size, &dma_handle, GFP_KERNEL);
        if (!dma_buffer) {
            printk(KERN_ALERT "cma_dma_pl: Ошибка выделения DMA-памяти\n");
        }
        printk(KERN_INFO "cma_dma_pl: Выделено %zu байт DMA-памяти\n", allocated_size);
    }
    
    return 0;
}


static void __exit kmem_exit(void) {
    if (dma_buffer) {
        dma_free_coherent(memory_device, allocated_size, dma_buffer, dma_handle);
    }
    device_destroy(memory_class, MKDEV(majorNumber, 0));
    class_unregister(memory_class);
    class_destroy(memory_class);
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "cma_dma_pl: Драйвер выгружен\n");
}



module_init(kmem_init); 
module_exit(kmem_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oorzhak Naydan");
MODULE_DESCRIPTION("Драйвер для выделение непрерывной области памяти");
MODULE_VERSION("1.1");

