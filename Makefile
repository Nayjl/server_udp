SRC_DIR = src
BUILD_DIR = build
TARGET ?= $(notdir $(abspath .)).elf

# Флаги компилятора
CC = arm-linux-gnueabihf-gcc
CFLAGS += -I$(SRC_DIR) -Wall -Wextra -g -MMD -MP
LIBS = #-lm

# Поиск всех C-файлов в src и подкаталогах
SOURCES := $(shell find $(SRC_DIR) -type f -name '*.c')
# Преобразование путей исходников в пути объектных файлов
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
# Файлы зависимостей
DEPS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.d, $(SOURCES))


DEBUG_USER ?= root
DEBUG_IP ?= 192.168.1.37
DEBUG_PORT ?= 2345
DEBUG_PATH ?= /mnt/data



# Включение файлов зависимостей
-include $(DEPS)


all: $(TARGET)


# Правило для линковки
$(TARGET): build_obj
	@echo "Линковка $@..."
	$(CC) $(OBJECTS) -o $@ $(LIBS)
	@echo "Готово: $@"


# Правило для компиляции C-файлов
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)  # Создание подкаталогов в build
	@echo "Компиляция $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@


# Цель чтобы собрать только объектные файлы *.o
build_obj: $(OBJECTS) 


# Очистка проекта
clean:
	@echo "Удаление объектных файлов и целевого файла..."
	@rm -rfv $(BUILD_DIR)/* *.elf 
	@echo "Очистка завершена"


run_file_device: copy_file_device
	@echo "Запуск файла на целевую плату..."
	ssh $(DEBUG_USER)@$(DEBUG_IP) "$(DEBUG_PATH)/$(TARGET)"

copy_file_device: del_file_device $(TARGET)
	@echo "Копирование файла на целевую плату..."
	scp $(TARGET) $(DEBUG_USER)@$(DEBUG_IP):$(DEBUG_PATH)/$(TARGET)


del_file_device: $(TARGET)
	@echo "Запуск файла на целевую плату..."
	ssh $(DEBUG_USER)@$(DEBUG_IP) "rm -fv $(DEBUG_PATH)/$(TARGET)"
	

# Запуск отладчика GDB для локальной отладки (если вы используете QEMU)
gdb: $(TARGET)
	arm-linux-gnueabihf-gdb $(TARGET)

# Запуск отладчика с подключением к удаленному gdbserver на Zynq
debug: $(TARGET)
	# @echo "Копирование файла на целевую плату..."
	# scp $(TARGET) $(DEBUG_USER)@$(DEBUG_IP):$(DEBUG_PATH)
	@echo "Запуск GDB для удаленной отладки..."
	arm-linux-gnueabihf-gdb $(TARGET) -ex "target remote $(DEBUG_IP):$(DEBUG_PORT)"

# Запуск gdbserver на целевой плате через SSH
debug_run: $(TARGET)
	scp $(TARGET) $(DEBUG_USER)@$(DEBUG_IP):$(DEBUG_PATH)/$(TARGET)
	ssh $(DEBUG_USER)@$(DEBUG_IP) "gdbserver :$(DEBUG_PORT) $(DEBUG_PATH)/$(TARGET)"

.PHONY: all clean
