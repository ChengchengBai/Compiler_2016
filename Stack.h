#pragma once
/******************************/
/*							  */
/*	动态栈实现			      */
/*	对应文件Stack.cpp	  	  */
/******************************/

typedef struct Stack
{
	void** base;
	void** top;
	int stacksize;
}Stack;

void init_stack(Stack* stack, int initsize);		//栈初始化
void* push(Stack* stack, void * e, int size);		//元素入栈
void pop(Stack* stack);								//	弹出栈顶
void * get_top(Stack* stack);						//得到栈顶元素
int is_empty(Stack* stack);							//栈非空
void stack_destroy(Stack* stack);					//栈销毁
