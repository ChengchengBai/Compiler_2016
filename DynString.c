/*
		DynString.cpp
		��̬�ַ�������ʵ��
		���ڴʷ����������ݽṹ
*/

#include<stdio.h>
#include<stdlib.h>
#include"Lex.h"


void dynstring_init(DynString* ptr, int initsize)			//��̬�ַ�����ʼ��
{
	if (ptr != NULL)
	{
		ptr->data = (char*)malloc(sizeof(char)*initsize);
		ptr->count = 0;
		ptr->capacity = initsize;
	}
}

void dynstring_free(DynString* ptr)							//�ͷŶ�̬�ַ���
{
	if (ptr != NULL)
	{
		if (ptr->data)
			free(ptr->data);
		ptr->count = 0;
		ptr->capacity = 0;
	}
}

void dynstring_reset(DynString* ptr)						//���³�ʼ��
{
	dynstring_free(ptr);
	dynstring_init(ptr, 8);
}

void dynstring_realloc(DynString* ptr, int new_size)		//���·�������
{
	int capacity;
	char* data;

	capacity = ptr->capacity;
	while (capacity<new_size)
	{
		capacity *= 2;
	}
	data = (char*)realloc(ptr->data, capacity);
	if (!data)
	{
		error("�޷������ڴ棡");
	}
	ptr->capacity = capacity;
	ptr->data = data;
}

void dynstring_chcat(DynString* ptr, char ch)				//׷�ӵ����ַ�����̬�ַ���
{
	int count;
	count = ptr->count + 1;
	if (count > ptr->capacity)
		dynstring_realloc(ptr, count);
	((char *)ptr->data)[count - 1] = ch;
	ptr->count = count;
}