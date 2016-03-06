/*
	DynArray.cpp
	��̬�������ݽṹ���
	���ڴʷ����������ݽṹ
*/
#include<stdio.h>
#include<stdlib.h>
#include"Lex.h"

void dynarray_init(DynArray* ptr, int initsize)				//��ʼ��
{
	if (ptr != NULL)
	{
		ptr->data = (void**)malloc(sizeof(void*)*initsize);
		ptr->capacity = initsize;
		ptr->count = 0;
	}
}

void dynarray_realloc(DynArray* ptr, int new_size)			//���·��䶯̬��������
{
	int capacity;
	void* data;

	capacity = ptr->capacity;
	while (capacity<new_size)
	{
		capacity *= 2;
	}
	data = realloc(ptr->data, capacity);
	if (!data)
		error("�ڴ����ʧ�ܣ�");
	ptr->capacity = capacity;
	ptr->data = (void **)data;
}

void dynarray_add(DynArray* ptr, void* data)				//׷�Ӷ�̬����Ԫ��
{
	int count;
	count = ptr->count + 1;
	if (count*sizeof(void*) > ptr->capacity)
	{
		dynarray_realloc(ptr, count*sizeof(void *));
	}
	ptr->data[count - 1] = data;
	ptr->count = count;
}

void dynarry_free(DynArray* ptr)				//�ڴ��ͷ�
{
	void** p;
	for (p = ptr->data; ptr->count;)
	{
		if (*p)
			free(*p);
		++p;
		--ptr->count;
	}
	free(ptr->data);
	ptr->data = NULL;
}

int dynarray_search(DynArray* ptr, int no)		 //��̬����Ԫ�ز���
{
	int i;
	int**p;
	p = (int**)ptr->data;
	for (i = 0; i < ptr->count; i++)
	{
		if (no == **p)
			return i;
	}
	return -1;
}