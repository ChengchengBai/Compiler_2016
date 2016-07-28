/******************************/
/*							  */
/*	���ű�ʵ��			      */
/*						  	  */
/******************************/

#include<stdio.h>
#include"Lex.h"
#include"Stack.h"
#include"SymbolTable.h"

Stack global_sym_stack;	//ȫ�ַ��ű�ջ
Stack local_sym_stack;	//�ֲ����ű�ջ
Type char_ptr_type;		//�ַ���ָ��
Type int_type;			//int����
Type default_func_type;	//Ĭ�Ϻ�������

extern DynArray tktable;	//���ʱ���ñ�ʶ��
extern int token;			//���ʱ���


Symbol* sym_direct_push(Stack* ss, int v, Type *t, int c)		//����ֱ����ջ��v���ű�ţ�c���Ź���ֵ
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

Symbol* sym_push(int v, Type *t, int r, int c)			//��������ջ����̬�ж���ȫ�ַ���ջ���Ǿֲ�����ջ��r���Ŵ洢����
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
	if ((v&BC_STRUCT) || v < BC_ANOM)			//����sym_struct��sym_identifier�ֶ�
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

Symbol* func_sym_push(int v, Type *t)			//���������ŷ���ȫ�ַ��ű�
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
	if ((r&BC_VALMASK) == BC_LOCAL)				//�ֲ�����
	{
		sym = sym_push(v, t, r, addr);
	}
	else if (v && (r&BC_VALMASK) == BC_GLOBAL)
	{
		sym = sym_search(v);
		if (sym)
			error("%s �ض���\n", ((TkWord*)tktable.data[v])->str);
		else
		{
			sym = sym_push(v, t, r | BC_SYM, 0);
		}
	}
	return sym;
}

Symbol* sec_sym_put(char * sec, int c)		//�������Ʒ�����ű�
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

void sym_pop(Stack *ptop, Symbol *b)			//����ջ�з���ֱ��ջ��Ϊb
{
	Symbol *s, **ps;
	TkWord *ts;
	int v;

	s = (Symbol*)get_top(ptop);
	while (s != b)
	{
		v = s->v;
			//���µ��ʱ��е�sym_struct sym_identifier
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

Symbol* struct_search(int v)		//���ҽṹ����
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