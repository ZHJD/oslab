#include <stdio.h>

// 内联汇编中，汇编语言引用的变量必须是全局变量
int a 	= 1;
int b 	= 2;
int sum = 0;

int main(void)
{
	asm volatile("pusha;" \
	"movl a, %eax;"		 \
	"movl b, %ebx;"		\
	"addl %eax, %ebx;" \
	"movl %ebx, sum;"  \
	"popa");
	printf("%d\n", sum);
	return 0;
}
