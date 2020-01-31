#include "print.h"
int main(void) {
	put_str("I am a kernel.\n");
	put_int(0);
	put_int(1);
	put_char('\n');
	put_int(0x000f);
	put_char('\n');
	put_int(0x01234);
	while(1);
	return 0;
}
