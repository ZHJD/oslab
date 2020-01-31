// 演示寄存器约束
#include <stdint.h>

int main(void)
{
	int a 	= 1;
	int b	 = 2;
	int sum = 0;
	// "a" 表示eax， “b”表示ebx
	asm volatile("addl %%ebx, %%eax": "=a"(sum): "a"(a), "b"(b));
	printf("%d\n", sum);
	return 0;
}
