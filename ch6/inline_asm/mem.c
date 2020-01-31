#include <stdint.h>
int main(void)
{
	int a = 1;
	int b = 2;
	printf("b is %d\n", b);
	// m是内存约束， %b0表示32位数据的低8位
	// %1序号占位符，为b所在的内存地址
	asm volatile ("movb %b0, %1": : "a"(a), "m"(b));
	printf("now b is %d\n", b);
	return 0;
}
