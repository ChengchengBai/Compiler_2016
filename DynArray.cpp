/*
	DynArray.cpp
	动态数组数据结构设计
	用于词法分析的数据结构
*/
#include<stdio.h>
#include<stdlib.h>
#include"Lex.h"

void dynarray_init(DynArray* ptr, int initsize)				//初始化
{
	if (ptr != NULL)
	{
		ptr->data = (void**)malloc(sizeof(void*)*initsize);
		ptr->capacity = initsize;
		ptr->count = 0;
	}
}

void dynarray_realloc(DynArray* ptr, int new_size)			//重新分配动态数组容量
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
		error("内存分配失败！");
	ptr->capacity = capacity;
	ptr->data = (void **)data;
}

void dynarray_add(DynArray* ptr, void* data)				//追加动态数组元素
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

void dynarry_free(DynArray* ptr)				//内存释放
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

int dynarray_search(DynArray* ptr, int no)		 //动态数组元素查找
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