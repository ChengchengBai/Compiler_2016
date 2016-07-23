/*
		TokenTable.cpp
		单词表的相关操作
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"Lex.h"

extern DynArray tktable;				//单词表放置标识符
extern TkWord* tk_hashtable[MAXKEY];	//单词哈希表
extern int token;						//单词编码

TkWord * tkWord_direct_insert(TkWord* tp)			//关键字，运算符直接放入单词表
{
	int keyno;
	tp->sym_identifier = NULL;
	tp->sym_struct = NULL;
	dynarray_add(&tktable, tp);
	keyno = elf_hash(tp->str);
	tp->next = tk_hashtable[keyno];
	tk_hashtable[keyno] = tp;
	return tp;
}

TkWord * tkword_search(char* p, int keyno)			//在单词表中查找单词
{
	TkWord* tp = NULL, *tp1;
	for (tp1 = tk_hashtable[keyno]; tp1; tp1 = tp1->next)
	{
		if (!strcmp(p, tp1->str))
		{
			token = tp1->tkcode;
			tp = tp1;
		}
	}
	return tp;
}

TkWord* tkword_insert(char* p)			//标识符插入单词表
{
	TkWord *tp;
	int keyno;
	char* s;
	char* end;
	int length;

	keyno = elf_hash(p);
	tp = tkword_search(p, keyno);
	if (tp == NULL)
	{
		length = strlen(p);
		tp = (TkWord*)mallocz(sizeof(TkWord)+length + 1);
		tp->next = tk_hashtable[keyno];
		tk_hashtable[keyno] = tp;
		dynarray_add(&tktable, tp);
		tp->tkcode = tktable.count - 1;
		s = (char*)tp + sizeof(TkWord);
		tp->str = (char*)s;
		for (end = p + length; p < end;)
		{
			*s++ = *p++;
		}
		*s = (char)'\0';

		tp->sym_identifier = NULL;
		tp->sym_struct = NULL;
	}
	return tp;
}

void *mallocz(int size)					//分配内存
{
	void* ptr;
	ptr = malloc(size);
	if (!ptr&&size)
	{
		error("内存分配失败");
	}
	memset(ptr, 0, size);
	return ptr;
}