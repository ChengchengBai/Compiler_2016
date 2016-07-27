#pragma once
/******************************/
/*							  */
/*	语法分析程序		      */
/*	对应文件Parsing.cpp  	  */
/******************************/

#include"SymbolTable.h"
#include"OutCoff.h"

/*
	用于语法缩紧
*/
extern int syntax_state;	//语法状态
extern int syntax_level;	//缩紧级别

enum  e_SyntaxState
{
	SNTX_NULL,		//空状态
	SNTX_SP,		//空格
	SNTX_LF_HT,		//换行并缩进，每一个声明、函数定义、语句结束都置为此状态
	SNTX_DELAY		//延迟到取下一个单词后确定输出格式
};

void parsing();				//语法分析单元
void external_decl(int b);	//解析外部声明		
int type_spec(Type *type);			//类型区分符
void struct_spec(Type *type);
void struct_decl_list(Type *type);
int calc_align(int n, int align);		//计算字节对齐位置
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
int type_size(Type *t, int *a);		//返回类型长度
void postfix_expression();
void primary_expression();
void argument_expression_list();
void syntax_indent();
void print_tab(int n);
