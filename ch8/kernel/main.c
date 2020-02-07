#include "print.h"
#include "init.h"
#include "memory.h"

int main(void) {
    put_str("I am a kernel.\n");
	init_all();
    void* vaddr = get_kernel_pages(4);
    put_str("get kernel pages start virtual address:0x");
    put_int((uint32_t)vaddr);
    put_char('\n');
   // asm volatile("sti"); // 开中断
	while(1);
	return 0;
}
