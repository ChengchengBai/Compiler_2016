/*
		TokenTable.cpp
		���ʱ����ز���
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"Lex.h"

TkWord * tkWord_direct_insert(TkWord* tp)			//�ؼ��֣������ֱ�ӷ��뵥�ʱ�
{
	int keyno;
	dynarray_add(&tktable, tp);
	keyno = elf_hash(tp->str);
	tp->next = tk_hashtable[keyno];
	tk_hashtable[keyno] = tp;
	return tp;
}

TkWord * tkword_search(char* p, int keyno)			//�ڵ��ʱ��в��ҵ���
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

TkWord* tkword_insert(char* p)			//��ʶ�����뵥�ʱ�
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
	}
	return tp;
}

void *mallocz(int size)					//�����ڴ�
{
	void* ptr;
	ptr = malloc(size);
	if (!ptr&&size)
	{
		error("�ڴ����ʧ��");
	}
	memset(ptr, 0, size);
	return ptr;
}