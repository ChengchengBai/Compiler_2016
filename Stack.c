/******************************/
/*							  */
/*	Stack.cpp			      */
/*	��̬ջʵ�֣����ڷ��ű� 	  */
/******************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"Lex.h"
#include"Stack.h"

void init_stack(Stack* stack, int initsize)		//ջ��ʼ��
{
	stack->base = (void**)malloc(sizeof(void**)*initsize);
	if (!stack->base)
	{
		error("�ڴ����ʧ�ܣ�");
	}
	else
	{
		stack->top = stack->base;
		stack->stacksize = initsize;
	}
}

void* push(Stack* stack, void* e, int size)		//Ԫ����ջ
{
	int newsize;
	if (stack->top >= stack->base + stack->stacksize)
	{
		newsize = stack->stacksize * 2;
		stack->base = (void**)realloc(stack->base, sizeof(void**)*newsize);
		if (!stack->base)
		{
			return NULL;
		}
		stack->top = stack->base + stack->stacksize;
		stack->stacksize = newsize;
	}
		*stack->top = (void**)malloc(size);
		memcpy(*stack->top, e, size);
		stack->top++;
		return *(stack->top - 1);
}

void pop(Stack* stack)		//	����ջ��
{
	if (stack->top > stack->base)
		free(*(--stack->top));
}

void * get_top(Stack* stack)
{
	void** e;
	if (stack->top > stack->base)
	{
		e = stack->top - 1;
		return *e;
	}
	else
	{
		return NULL;
	}
}

int is_empty(Stack* stack)
{
	if (stack->base == stack->top)
	{
		return 1;
	}
	else
		return 0;
}

void stack_destroy(Stack* stack)		//ջ����
{
	void **e;
	for (e = stack->base; e < stack->top; e++)
	{
		free(*e);
	}
	if (stack->base)
	{
		free(stack->base);
	}
	stack->base = NULL;
	stack->top = NULL;
	stack->stacksize = 0;
}