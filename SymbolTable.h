#pragma once
/******************************/
/*							  */
/*	符号表相关数据结构	      */
/*						  	  */
/******************************/
#include"Stack.h"

//extern Stack global_sym_stack;	//全局符号表栈
//extern Stack local_sym_stack;		//局部符号表栈
//extern Type char_ptr_type;		//字符串指针
//extern Type int_type;				//int类型
//extern Type default_func_type;	//默认函数类型

/*
数据结构类型
*/
typedef struct Type
{
	int t;					// 数据类型
	struct Symbol * ref;	//引用符号
}Type;


/*
符号表
对应文件 SymbolTable.cpp
*/
typedef struct Symbol
{
	int v;		//符号的单词编码
	int r;		//符号的存储类型
	int c;		//符号的关联值
	Type type;	//符号数据类型
	struct Symbol *next;		//相关联其他符号
	struct Symbol *prev_tok;		//指向前一定义的同名符号
}Symbol;



Symbol* sym_direct_push(Stack *ss, int v, Type *t, int c);		//符号直接入栈，v符号编号，c符号关联值
Symbol* sym_push(int v, Type *t, int r, int c);					//将符号入栈，动态判断是全局符号栈还是局部符号栈，r符号存储类型
Symbol* func_sym_push(int v, Type *t);							//将函数符号放入全局符号表
Symbol* var_sym_put(Type *t, int r, int v, int addr);
Symbol* sec_sym_put(char *sec, int c);							//将节名称放入符号表
void sym_pop(Stack *ptop, Symbol *b);							//弹出栈中符号直到栈顶为b
Symbol* struct_search(int v);									//查找结构定义
Symbol* sym_search(int v);


/*	数据类型编码	*/

enum e_TypeCode
{
	T_INT,
	T_CHAR,
	T_SHORT,
	T_VOID,
	T_PTR,
	T_FUNC,
	T_STRUCT,

	T_BTYPE = 0x000f,	//基本类型掩码
	T_ARRAY = 0x0010,	//数组	
};

/*	存储类型	*/
enum e_StorageClass
{
	BC_GLOBAL = 0x00f0,		//包括整型常量，字符常量，字符串常量，全局变量，函数定义
	BC_LOCAL = 0x00f1,		//栈中变量
	BC_LLOCAL = 0x00f2,		//寄存器溢出存放栈中
	BC_CMP = 0x00f3,		//使用标志寄存器
	BC_VALMASK = 0x0ff,		//存储类型掩码
	BC_LVAL = 0x0100, 		//左值
	BC_SYM = 0x0200,			//符号

	BC_ANOM = 0x10000000,	//匿名符号
	BC_STRUCT = 0x20000000,	//结构体符号
	BC_MEMBER = 0x40000000,	//结构体成员变量
    BC_PARAMS = 0x80000000,	//函数参数
};