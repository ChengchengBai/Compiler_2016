/*
		DynString.cpp
		动态字符串功能实现
		用于词法分析的数据结构
*/

#include<stdio.h>
#include<stdlib.h>
#include"Lex.h"


void dynstring_init(DynString* ptr, int initsize)			//动态字符串初始化
{
	if (ptr != NULL)
	{
		ptr->data = (char*)malloc(sizeof(char)*initsize);
		ptr->count = 0;
		ptr->capacity = initsize;
	}
}

void dynstring_free(DynString* ptr)							//释放动态字符串
{
	if (ptr != NULL)
	{
		if (ptr->data)
			free(ptr->data);
		ptr->count = 0;
		ptr->capacity = 0;
	}
}

void dynstring_reset(DynString* ptr)						//重新初始化
{
	dynstring_free(ptr);
	dynstring_init(ptr, 8);
}

void dynstring_realloc(DynString* ptr, int new_size)		//重新分配容量
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
		error("无法分配内存！");
	}
	ptr->capacity = capacity;
	ptr->data = data;
}

void dynstring_chcat(DynString* ptr, char ch)				//追加单个字符到动态字符串
{
	int count;
	count = ptr->count + 1;
	if (count > ptr->capacity)
		dynstring_realloc(ptr, count);
	((char *)ptr->data)[count - 1] = ch;
	ptr->count = count;
}