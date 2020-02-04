#include "print.h"

void put_str(const char* message)
{
	int i = 0;
	while(message[i] != '\0')
	{
		put_char(message[i++]);
	}
}
