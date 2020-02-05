BUILD_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I ./lib -I ./lib/kernel -I ./kernel -I ./device
ASFLAGS = -f elf32
CFLAGS = -m32 -Wall $(LIB) -c -fno-builtin -fno-stack-protector
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
			 $(BUILD_DIR)/print.o $(BUILD_DIR)/kernel.o \
			 $(BUILD_DIR)/timer.o $(BUILD_DIR)/debug.o \
			 $(BUILD_DIR)/memory.o

###################   C代码编译  ########################################
$(BUILD_DIR)/main.o: kernel/main.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: kernel/memory.c
	$(CC) $(BUILD_DIR) $< -o $@

$(BUILD_DIR)/debug.o: kernel/debug.c
	$(CC) $(CFLAGS) $< -o $@

#################   汇编代码编译  ######################################
$(BUILD_DIR)/kernel.o: kernel/kernel.S
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD_DIR)/print.o: lib/kernel/print.S
	$(AS) $(ASFLAGS) $< -o $@

################   链接所有目标文件 ###################################
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

all:$(BUILD_DIR)/kernel.bin
	@echo "编译完成"

clean:
	cd ./$(BUILD_DIR)/ && rm *
