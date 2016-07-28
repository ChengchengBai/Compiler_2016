/******************************/
/*							  */
/*	符号表实现			      */
/*						  	  */
/******************************/

#include<stdio.h>
#include"Lex.h"
#include"Stack.h"
#include"SymbolTable.h"

Stack global_sym_stack;	//全局符号表栈
Stack local_sym_stack;	//局部符号表栈
Type char_ptr_type;		//字符串指针
Type int_type;			//int类型
Type default_func_type;	//默认函数类型

extern DynArray tktable;	//单词表放置标识符
extern int token;			//单词编码


Symbol* sym_direct_push(Stack* ss, int v, Type *t, int c)		//符号直接入栈，v符号编号，c符号关联值
{
	Symbol s, *p;
	s.v = v;
	s.type.ref = t->ref;
	s.type.t = t->t;
	s.c = c;
	s.next = NULL;
	p = (Symbol*)push(ss, &s, sizeof(Symbol));
	return p;
}

Symbol* sym_push(int v, Type *t, int r, int c)			//将符号入栈，动态判断是全局符号栈还是局部符号栈，r符号存储类型
{
	Symbol *ps, **pps;
	TkWord *ts;
	Stack *ss;

	if (is_empty(&local_sym_stack) == 0)
	{
		ss = &local_sym_stack;
	}
	else
	{
		ss = &global_sym_stack;
	}
	ps = sym_direct_push(ss, v, t, c);
	ps->r = r;
	if ((v&BC_STRUCT) || v < BC_ANOM)			//更新sym_struct或sym_identifier字段
	{
		ts = (TkWord*)tktable.data[(v&~BC_STRUCT)];
		if (v&BC_STRUCT)
			pps = &ts->sym_struct;
		else
			pps = &ts->sym_identifier;
		ps->prev_tok = *pps;
		*pps = ps;
	}
	return ps;
}

Symbol* func_sym_push(int v, Type *t)			//将函数符号放入全局符号表
{
	Symbol *s, **ps;
	s = sym_direct_push(&global_sym_stack, v, t, 0);

	ps = &((TkWord*)tktable.data[v])->sym_identifier;

	while (*ps!=NULL)
	{
		ps = &(*ps)->prev_tok;
	}
	s->prev_tok = NULL;
	*ps = s;
	return s;
}

Symbol* var_sym_put(Type *t, int r, int v, int addr)
{
	Symbol *sym = NULL;
	if ((r&BC_VALMASK) == BC_LOCAL)				//局部变量
	{
		sym = sym_push(v, t, r, addr);
	}
	else if (v && (r&BC_VALMASK) == BC_GLOBAL)
	{
		sym = sym_search(v);
		if (sym)
			error("%s 重定义\n", ((TkWord*)tktable.data[v])->str);
		else
		{
			sym = sym_push(v, t, r | BC_SYM, 0);
		}
	}
	return sym;
}

Symbol* sec_sym_put(char * sec, int c)		//将节名称放入符号表
{
	TkWord *tp;
	Symbol *s;
	Type t;
	t.t = T_INT;
	tp = tkword_insert(sec);
	token = tp->tkcode;
	s = sym_push(token, &t, BC_GLOBAL, c);
	return s;
}

void sym_pop(Stack *ptop, Symbol *b)			//弹出栈中符号直到栈顶为b
{
	Symbol *s, **ps;
	TkWord *ts;
	int v;

	s = (Symbol*)get_top(ptop);
	while (s != b)
	{
		v = s->v;
			//更新单词表中的sym_struct sym_identifier
		if ((v&BC_STRUCT) || v < BC_ANOM)
		{
			ts = (TkWord*)tktable.data[(v&~BC_STRUCT)];
			if (v&BC_STRUCT)
				ps = &ts->sym_struct;
			else
				ps = &ts->sym_identifier;
			*ps = s->prev_tok;
		}
		pop(ptop);
		s = (Symbol*)get_top(ptop);
	}
}

Symbol* struct_search(int v)		//查找结构定义
{
	if (v >= tktable.count)
		return NULL;
	else
		return ((TkWord*)tktable.data[v])->sym_struct;
}

Symbol* sym_search(int v)
{
	if (v >= tktable.count)
		return NULL;
	else
		return ((TkWord*)tktable.data[v])->sym_identifier;
}