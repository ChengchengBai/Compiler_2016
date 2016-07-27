#pragma once
/******************************/
/*							  */
/*	�﷨��������		      */
/*	��Ӧ�ļ�Parsing.cpp  	  */
/******************************/

#include"SymbolTable.h"
#include"OutCoff.h"

/*
	�����﷨����
*/
extern int syntax_state;	//�﷨״̬
extern int syntax_level;	//��������

enum  e_SyntaxState
{
	SNTX_NULL,		//��״̬
	SNTX_SP,		//�ո�
	SNTX_LF_HT,		//���в�������ÿһ���������������塢����������Ϊ��״̬
	SNTX_DELAY		//�ӳٵ�ȡ��һ�����ʺ�ȷ�������ʽ
};

void parsing();				//�﷨������Ԫ
void external_decl(int b);	//�����ⲿ����		
int type_spec(Type *type);			//�������ַ�
void struct_spec(Type *type);
void struct_decl_list(Type *type);
int calc_align(int n, int align);		//�����ֽڶ���λ��
void struct_decl(int *maxalign, int *offset, Symbol ***ps);
void function_call_convention(int* fc);
void struct_mem_align(int *force_align);
void declarator(Type *type,int *v,int *force_align);
void direct_declarator(Type *type,int *v, int func_call);
void direct_declarator_postfix(Type *type,int func_call);
void parameter_type_list(Type *type, int func_call);
void funcbody(Symbol *sym);
void initializer(Type *type,int c, Section *sec);
void statement(int *bsym,int *csym);
void compound_statement(int *bsym,int *csym);
int is_type_spec(int v);
void expression_statement();
void if_statement(int *bsym, int *csym);
void for_statement(int *bsym, int *csym);
void continue_statement(int *csym);
void break_statement(int *bsym);
void return_statement();
void expression();
void assignment_expression();
void equality_expression();
void relation_expression();
void addtive_expression();
void multuplicative_expresstion();
void unary_expression();
void sizeof_expression();
int type_size(Type *t, int *a);		//�������ͳ���
void postfix_expression();
void primary_expression();
void argument_expression_list();
void syntax_indent();
void print_tab(int n);
