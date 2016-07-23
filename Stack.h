#pragma once
/******************************/
/*							  */
/*	��̬ջʵ��			      */
/*	��Ӧ�ļ�Stack.cpp	  	  */
/******************************/

typedef struct Stack
{
	void** base;
	void** top;
	int stacksize;
}Stack;

void init_stack(Stack* stack, int initsize);		//ջ��ʼ��
void* push(Stack* stack, void * e, int size);		//Ԫ����ջ
void pop(Stack* stack);								//	����ջ��
void * get_top(Stack* stack);						//�õ�ջ��Ԫ��
int is_empty(Stack* stack);							//ջ�ǿ�
void stack_destroy(Stack* stack);					//ջ����
