# 把loader.bin 写入硬盘映像
dd if=./ch5/boot/loader.bin of=./hd60M.img bs=512 count=4 seek=1 conv=notrunc       
