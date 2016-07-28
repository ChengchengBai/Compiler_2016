#pragma once
/******************************/
/*							  */
/*	���ű�������ݽṹ	      */
/*						  	  */
/******************************/
#include"Stack.h"

//extern Stack global_sym_stack;	//ȫ�ַ��ű�ջ
//extern Stack local_sym_stack;		//�ֲ����ű�ջ
//extern Type char_ptr_type;		//�ַ���ָ��
//extern Type int_type;				//int����
//extern Type default_func_type;	//Ĭ�Ϻ�������

/*
���ݽṹ����
*/
typedef struct Type
{
	int t;					// ��������
	struct Symbol * ref;	//���÷���
}Type;


/*
���ű�
��Ӧ�ļ� SymbolTable.cpp
*/
typedef struct Symbol
{
	int v;		//���ŵĵ��ʱ���
	int r;		//���ŵĴ洢����
	int c;		//���ŵĹ���ֵ
	Type type;	//������������
	struct Symbol *next;		//�������������
	struct Symbol *prev_tok;		//ָ��ǰһ�����ͬ������
}Symbol;



Symbol* sym_direct_push(Stack *ss, int v, Type *t, int c);		//����ֱ����ջ��v���ű�ţ�c���Ź���ֵ
Symbol* sym_push(int v, Type *t, int r, int c);					//��������ջ����̬�ж���ȫ�ַ���ջ���Ǿֲ�����ջ��r���Ŵ洢����
Symbol* func_sym_push(int v, Type *t);							//���������ŷ���ȫ�ַ��ű�
Symbol* var_sym_put(Type *t, int r, int v, int addr);
Symbol* sec_sym_put(char *sec, int c);							//�������Ʒ�����ű�
void sym_pop(Stack *ptop, Symbol *b);							//����ջ�з���ֱ��ջ��Ϊb
Symbol* struct_search(int v);									//���ҽṹ����
Symbol* sym_search(int v);


/*	�������ͱ���	*/

enum e_TypeCode
{
	T_INT,
	T_CHAR,
	T_SHORT,
	T_VOID,
	T_PTR,
	T_FUNC,
	T_STRUCT,

	T_BTYPE = 0x000f,	//������������
	T_ARRAY = 0x0010,	//����	
};

/*	�洢����	*/
enum e_StorageClass
{
	BC_GLOBAL = 0x00f0,		//�������ͳ������ַ��������ַ���������ȫ�ֱ�������������
	BC_LOCAL = 0x00f1,		//ջ�б���
	BC_LLOCAL = 0x00f2,		//�Ĵ���������ջ��
	BC_CMP = 0x00f3,		//ʹ�ñ�־�Ĵ���
	BC_VALMASK = 0x0ff,		//�洢��������
	BC_LVAL = 0x0100, 		//��ֵ
	BC_SYM = 0x0200,			//����

	BC_ANOM = 0x10000000,	//��������
	BC_STRUCT = 0x20000000,	//�ṹ�����
	BC_MEMBER = 0x40000000,	//�ṹ���Ա����
    BC_PARAMS = 0x80000000,	//��������
};