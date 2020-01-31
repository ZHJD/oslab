char *str = "hello world\n";
int count = 0;
int len = 12;
int main()
{
	// 备份8个数据寄存器
    // 4号系统调用write
	// 1表示标准输出
    // 字符串首地址
	// 字符串长度
    // 返回值
	// 恢复寄存器
	asm volatile("pusha;" \ 
	"movl $4, %eax;"	\
	"movl $1, %ebx;"   \	 
	"movl str, %ecx;" \		
	"movl len, %edx;" \	
	"int $ 0x80;"		 \	 
	"mov %eax, count;"\
	"popa"					
	);
	return 0;
}
