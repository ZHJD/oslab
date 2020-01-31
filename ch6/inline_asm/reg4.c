#include <stdio.h>

int main(void)
{
	int in_a = 0x12345678;
	int in_b = 0;
	// movw 传诵低16位
	asm volatile("movw %1, %0": "=m"(in_b) : "a"(in_a));

	printf("now word b is 0x%x.\n", in_b);

	in_b = 0;
	asm volatile("movb %1, %0" : "=m"(in_b) : "a"(in_a));

	printf("low byte b is 0x%x.\n", in_b);

	in_b = 0;
	%h1,表示传诵低2字节中的高8位
	asm volatile("movb %h1, %0": "=m"(in_b) : "a"(in_a));
	printf("high byte is 0x%x.\n", in_b);
	return 0;
}
