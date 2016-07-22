/*
	¹şÏ£Öµ¼ÆËã
*/
#include"Lex.h"

int elf_hash(char* key)
{
	unsigned int hash = 0, x = 0;
	while (*key)
	{
		hash = (hash << 4) + (*key++);
		if ((x=hash&0xf0000000)!=0)
		{
			hash ^= x >> 24;
			hash&~x;
		}
	}
	return hash%MAXKEY;
}