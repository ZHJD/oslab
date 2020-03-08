BUILD_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I ./lib -I ./lib/string -I ./lib/kernel -I ./kernel -I ./device \
	  -I ./kernel/thread -I kernel/semaphore/ -I ./userprocess/ -I ./lib/user \
	  -I ./fs/

ASFLAGS = -f elf32
CFLAGS = -m32 -Wall $(LIB) -c -fno-builtin -fno-stack-protector
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
			 $(BUILD_DIR)/print.o $(BUILD_DIR)/kernel.o \
			 $(BUILD_DIR)/timer.o $(BUILD_DIR)/debug.o \
			 $(BUILD_DIR)/memory.o $(BUILD_DIR)/bitmap.o \
			 $(BUILD_DIR)/string.o $(BUILD_DIR)/thread.o \
			 $(BUILD_DIR)/list.o $(BUILD_DIR)/switch_to.o \
			 $(BUILD_DIR)/set_cursor.o $(BUILD_DIR)/sync.o \
			 $(BUILD_DIR)/console.o $(BUILD_DIR)/keyboard.o \
			 $(BUILD_DIR)/semaphore.o $(BUILD_DIR)/ioqueue.o \
			 $(BUILD_DIR)/tss.o $(BUILD_DIR)/process.o       \
			 $(BUILD_DIR)/syscall_init.o $(BUILD_DIR)/syscall.o \
			 $(BUILD_DIR)/stdio.o $(BUILD_DIR)/ide.o 			\
			 $(BUILD_DIR)/stdio_kernel.o $(BUILD_DIR)/fs.o

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
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: kernel/debug.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: lib/kernel/bitmap.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/string.o: lib/string/string.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread.o: kernel/thread/thread.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/list.o: lib/kernel/list.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/sync.o: kernel/thread/sync.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/console.o: device/console.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: device/keyboard.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/semaphore.o: kernel/semaphore/semaphore.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ioqueue.o: device/ioqueue.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/tss.o: userprocess/tss.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/process.o:userprocess/process.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall_init.o: userprocess/syscall_init.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall.o: lib/user/syscall.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio.o: lib/stdio.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ide.o: device/ide.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio_kernel.o: lib/kernel/stdio_kernel.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/fs.o: fs/fs.c
	$(CC) $(CFLAGS) $< -o $@

#################   汇编代码编译  ######################################
$(BUILD_DIR)/kernel.o: kernel/kernel.S
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/print.o: lib/kernel/print.S
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/switch_to.o: kernel/thread/switch_to.S
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/set_cursor.o: lib/kernel/set_cursor.S
	$(AS) $(ASFLAGS) $< -o $@
################   链接所有目标文件 ###################################
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

all:$(BUILD_DIR)/kernel.bin
	@echo "kernel.bin 生成"

hd:
	@dd if=./build/kernel.bin of=./../hd60M.img bs=512 count=200 seek=9 conv=notrunc 

clean:
	@cd ./$(BUILD_DIR)/ && rm *
