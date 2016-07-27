/*
	Parsing.cpp
	�﷨��������
*/
#include<stdio.h>
#include<stdlib.h>
#include"Lex.h"
#include"BCC.h"
#include"Parsing.h"
#include"SymbolTable.h"
#include"OutCoff.h"
#include"GenCode.h"

#define ALIGN_SET 0x100

int syntax_state;	//�﷨״̬
int syntax_level;	//��������

extern DynString tkstr;
extern int token;
extern int tkvalue;

extern Stack global_sym_stack;	//ȫ�ַ��ű�ջ
extern Stack local_sym_stack;		//�ֲ����ű�ջ
extern Type char_ptr_type;		//�ַ���ָ��
extern Type int_type;				//int����
extern Type default_func_type;	//Ĭ�Ϻ�������

extern int rsym;
extern int ind;			//ָ���ڴ���ڵ�λ��
extern Section *sec_text;			//�����

extern Operand *optop;		

void parsing()				//�﷨������Ԫ
{
	while (token != TK_EOF)
	{
		external_decl(BC_GLOBAL);
	}
}

/***********************************************************************
	�ⲿ��������
	�ķ�Ϊ��
	<external_decl>-><type_spec>(<TK_SEMICOLON>|<declarator><function_body>|
	<declarator>[<TK_ASSIGN><initializer>]
	{<TK_COMMA><declarator>[TK_ASSIGN<initializer>]}<TK_SEMI>)
***********************************************************************/

void external_decl(int b)	//�����ⲿ����		
{
	Type btype, type;
	int v, has_init, r, addr = 0;
	Symbol * sym;
	Section *sec = NULL;

	if (!type_spec(&btype))
	{
		expect("<�������ַ�>");
	}
	if (btype.t == T_STRUCT && token == TK_SEMICILON)
	{
		get_token();
		return;
	}
	while (1)
	{
		type = btype;
		declarator(&type,&v,NULL);

		if (token == TK_BEGIN)		//��������
		{
			if (b == BC_LOCAL)
				error("��֧�ֺ���Ƕ�׶���");

			if ((type.t&T_BTYPE) != T_FUNC)
				expect("<��������>");
		
			sym = sym_search(v);
			if (sym)			//������������
			{
				if ((sym->type.t&T_BTYPE) != T_FUNC)
					error("'%s'�ض���", get_tkstr(v));
				sym->type = type;
			}
			else
			{
				sym = func_sym_push(v, &type);
			}
			sym->r = BC_SYM | BC_GLOBAL;
			funcbody(sym);
			break;
		}
		else
		{
			if ((type.t&T_BTYPE) == T_FUNC)		//��������
			{
				if (sym_search(v) == NULL)
				{
					sym = sym_push(v, &type, BC_GLOBAL | BC_SYM, 0);
				}
			}
			else
			{						//��������
				r = 0;
				if (!(type.t&T_ARRAY))
					r |= BC_LVAL;

				r |= b;
				has_init=(token == TK_ASSIN);

				if (has_init)
				{
					get_token();
				}
				sec = allocate_storage(&type, r, has_init, v, &addr);
				sym = var_sym_put(&type, r, v, addr);
				if (b == BC_GLOBAL)
					coffsym_add_update(sym, addr, sec->index, 0, IMAGE_SYM_CLASS_EXTERNAL);
				if (has_init)
				{
					initializer(&type, addr, sec);
				}
			}
			if (token == TK_COMMA)
			{
				get_token();
			}
			else
			{
				syntax_state = SNTX_LF_HT;
				skip(TK_SEMICILON);
				break;
			}
		}
	}
}

int type_spec(Type *type)				//�������ַ�
{
	int t = 0;
	Type type1;
	int type_found = 0;
	switch (token)
	{
	case KW_CHAR:
		t = T_CHAR;
		syntax_state = SNTX_SP;
		type_found = 1;
		get_token();
		break;
	case KW_SHORT:
		t = T_SHORT;
		syntax_state = SNTX_SP;
		type_found = 1;
		get_token();
		break;
	case KW_VOID:
		t = T_VOID;
		syntax_state = SNTX_SP;
		type_found = 1;
		get_token();
		break;
	case KW_INT:
		t = T_INT;
		syntax_state = SNTX_SP;
		type_found = 1;
		get_token();
		break;
	case KW_STRUCT:
		syntax_state = SNTX_SP;
		struct_spec(&type1);
		type->ref = type1.ref;
		t = T_STRUCT;
		type_found = 1;
		break;
	default:
		break;
	}
	type->t = t;
	return type_found;
}

/*
	���ֽṹ��
	<struct_spec>-><KW_STRUCT><IDENTIFIER><TK_BEGIN><struct_decl_list><TK_END>
	|<KW_STRUCT><INDENTIFIER>
*/
void struct_spec(Type *type)
{
	int v;
	Symbol *s;
	Type type1;

	get_token();
	v = token;

	syntax_state = SNTX_DELAY;
	get_token();

	if (token == TK_BEGIN)				//�ṹ�嶨�������
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA)
		syntax_state = SNTX_NULL;
	else
		syntax_state = SNTX_SP;
	syntax_indent();

	if (v < TK_IDENTI)			//�ؼ��ֲ�����Ϊ�ṹ����
		expect("�ṹ����");

	s = struct_search(v);
	if (!s)
	{
		type1.t = KW_STRUCT;
		s = sym_push(v | BC_STRUCT, &type1, 0, -1);
		s->r = 0;
	}

	type->t = T_STRUCT;
	type->ref = s;

	if (token == TK_BEGIN)
	{
		struct_decl_list(type);
	}
}

/*
	�����ṹ��������
	<struct_decl_list>-><struct_decl>{<struct_decl>}
	type(���)���ṹ����
*/
void struct_decl_list(Type *type)
{
	int maxalign, offset;
	Symbol *s, **ps;
	s = type->ref;

	syntax_state = SNTX_LF_HT;		//�ṹ���Ա����
	syntax_level++;

	get_token();
	if (s->c != -1)
		error("�ṹ���Ѷ���");
	maxalign = 1;
	ps = &s->next;
	offset = 0;

	while (token!=TK_END)
	{
		struct_decl(&maxalign, &offset, &ps);
	}
	skip(TK_END);

	syntax_state = SNTX_LF_HT;
	s->c = calc_align(offset, maxalign);		//�ṹ���С
	s->r = maxalign;							//�ṹ�����
}

int calc_align(int n, int align)		//�����ֽڶ���λ��
{
	return ((n + align - 1)&(~(align - 1)));
}

/*
	<struct_decl>-><type_spec><declarator>{<TK_COMMA><declaration>}<TK_SIMICOLON>
	maxalign���������������Ա����������
	offset������������ƫ����
	ps����������ṹ�������
*/
void struct_decl(int *maxalign, int *offset, Symbol ***ps)
{
	int v, size, align;
	Symbol *ss;
	Type type1, btype;
	int force_align;
	type_spec(&btype);
	while (1)
	{
		v = 0;
		type1 = btype;
		declarator(&type1, &v, &force_align);
		size = type_size(&type1, &align);
		if (force_align & ALIGN_SET)
			align = force_align&~ALIGN_SET;

		*offset = calc_align(*offset, align);

		if (align > *maxalign)
			*maxalign = align;
		ss = sym_push(v | BC_MEMBER, &type1, 0, *offset);
		*offset += size;
		**ps = ss;
		*ps = &ss->next;

		if (token == TK_SEMICILON)
			break;
		skip(TK_COMMA);
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICILON);
}

/*
	Ԥ������������Լ��
	<function_call_convention>-><KW_CDECL>|<KW_STDCALL>
	fc�������������Լ��
*/

void function_call_convention(int* fc)
{
	*fc = KW_CDECL;
	if (token == KW_CDECL || token == KW_STDCALL)
	{
		*fc = token;
		syntax_state = SNTX_SP;
		get_token();
	}
}

/*
	Ԥ�����ṹ���Ա����
	<struct_member_alignment>
	force_align���������ǿ�ƶ�������
*/
void struct_mem_align(int *force_align)
{
	int align = 1;
	if (token == KW_ALIGN)
	{
		get_token();
		skip(TK_OPENPA);
		if (token == TK_CINT)
		{
			get_token();
			align = tkvalue;
		}
		else
			expect("��������");
		skip(TK_CLOSEPA);
		if (align != 1 && align != 2 && align != 4)
			align = 1;
		align |= ALIGN_SET;
		*force_align = align;
	}
	else
		*force_align;
}

/*
	������
	<declarator>->{<TK_STAR>}[<function_call_convention>][<struct_member_alignment>]<direct_declarator>
	type����������
	v������������ʱ��
	force_align���������ǿ�ƶ�������
*/
void declarator(Type *type,int *v,int *force_align)
{
	int fc;
	while (token==TK_STAR)
	{
		mk_pointer(type);				//�ǹ���ָ����
		get_token();
	}
	function_call_convention(&fc);
	if (!force_align);
	struct_mem_align(force_align);
	direct_declarator(type, v, fc);
}

/*
	ֱ��������
	<direct_declarator>-><IDENTIFIER><direct_declarator_postfix>
	type�����룬���������������
	v������������ʱ��
	fun_call����������Լ��

	�ú�������Ҫ���������д��ں��ĵ�λ����Ϊ�����е��������ֶ��������������ʶ����
*/
void direct_declarator(Type *type,int *v, int func_call)
{
	if (token >= TK_IDENTI)
	{
		*v = token;
		get_token();
	}
	else
	{
		expect("��ʶ��");
	}
	direct_declarator_postfix(type,func_call);
}

/*
	ֱ��������׺��
	<direct_declarator_postfix>->{<TK_OPENBR><TK_CINT><TK_CLOSEBR>
	|<TK_OPENBR><TK_CLOSEBR>
	|<TK_OPENPA><parameter_type_list><TK_CLOSEPA>
	|<TK_OPENPA><TK_CLOSEPA>}

	type�����룬���������������
	fun_call����������Լ��
*/
void direct_declarator_postfix(Type *type,int func_call)
{
	int n;
	Symbol *s;

	if (token == TK_OPENPA)
	{
		parameter_type_list(type,func_call);
	}
	else if (token == TK_OPENBR)
	{
		get_token();
		n = -1;
		if (token = TK_CINT)
		{
			get_token(); 
			n = tkvalue;
		}
		skip(TK_CLOSEBR);
		direct_declarator_postfix(type,func_call);
		s = sym_push(BC_ANOM, type, 0, n);
		type->t = T_ARRAY | T_PTR;
		type->ref;
	}
}

/*
	�β����ͱ�
	<parameter_type_list>-><type_spec>{<declarator>}
	{<TK_COMMA><type_spec>{<declarator>}}<TK_COMMA><TK_ELLIPSIS>

	type�����룬���������������
	fun_call����������Լ��
*/
void parameter_type_list(Type *type, int func_call)
{
	int n;
	Symbol **plast, *s, *first;
	Type pt;

	get_token();
	first = NULL;
	plast = &first;

	while (token != TK_CLOSEPA)
	{
		if (!type_spec(&pt))
			error("��Ч���ͷ���");
		declarator(&pt,&n,NULL);
		s = sym_push(n|BC_PARAMS, &pt, 0,0);
		*plast = s;
		plast = &s->next;
		if (token == TK_CLOSEPA)
			break;
		skip(TK_COMMA);
		if (token == TK_ELLPI)
		{
			func_call = KW_CDECL;
			get_token();
			break;
		}
	}

	s = sym_push(BC_ANOM, type, func_call, 0);		//�������������ʹ洢��Ȼ��ָ��������type��Ϊ�������ͣ����õ������Ϣ
	s->next = first;								//
	type->t = T_FUNC;
	type->ref = s;

	syntax_state = SNTX_DELAY;
	skip(TK_CLOSEPA);
	if (token == TK_BEGIN)			//��������
		syntax_state = SNTX_LF_HT;
	else                           //��������
		syntax_state = SNTX_NULL;
	syntax_indent();
}

/*
	������
	<funcbody>-><compound_statement>
	sym����������
*/
void funcbody(Symbol *sym)
{
	ind = sec_text->data_offset;
	coffsym_add_update(sym, ind, sec_text->index, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);
	/* ��һ���������ھֲ����ű��� */
	sym_direct_push(&local_sym_stack, BC_ANOM, &int_type, 0);
	gen_prolog(&sym->type);
	rsym = 0;	
	compound_statement(NULL,NULL);
	backpatch(rsym, ind);
	gen_epilog();
	sec_text->data_offset = ind;
	sym_pop(&local_sym_stack, NULL);

}

/*
	��ֵ��
	<initializer>-><assignment_expression>
	type����������
*/
void initializer(Type *type,int c,Section *sec)
{
	if (type->t&T_ARRAY&&sec)
	{
		memcpy(sec->data + c, tkstr.data, tkstr.count);
		get_token();
	}
	else
	{
		assignment_expression();
		init_variable(type, sec, c, 0);
	}
}

/*
	���
	<statement>-><compound_statement>|<if_statement>|<return_statement>|<break_statement>|<continue_statement>
	|<for_statement>|<expression_statement>
	bsym��break ��תλ��
	csym��continue ��תλ��
*/
void statement(int *bsym,int *csym)
{
	switch (token)
	{
	case TK_BEGIN:
		compound_statement(bsym, csym);
		break;
	case KW_IF:
		if_statement(bsym,csym);
		break;
	case KW_RETURN:
		return_statement();
		break;
	case KW_BREAK:
		break_statement(bsym);
		break;
	case KW_CONTINUE:
		continue_statement(csym);
		break;
	case KW_FOR:
		for_statement(bsym,csym);
		break;
	default:
		expression_statement();
		break;
	}
}

/*
	�������
	<compound_statement>-><TK_BEGIN>{<declaration>}{<statement>}<TK_END>
	bsym��break ��תλ��
	csym��continue ��תλ��

	PS����ʵ��<declaration>��<statement>��˳����Խ���
	changed1

*/
void compound_statement(int *bsym,int *csym)
{
	syntax_state = SNTX_LF_HT;			//�������
	syntax_level++;

	Symbol *s;
	s = (Symbol*)get_top(&local_sym_stack);

	get_token();

	while (token!=TK_END)
	{
		while (is_type_spec(token))
		{
			external_decl(BC_LOCAL);
		}
		statement(bsym, csym);
	}
	syntax_state = SNTX_LF_HT;
	sym_pop(&local_sym_stack,s);
	get_token();
}

/*
	�ж��Ƿ�Ϊ�������ַ�
*/
int is_type_spec(int v)
{
	switch (v)
	{
	case KW_CHAR:
	case KW_SHORT:
	case KW_INT:
	case KW_VOID:
	case KW_STRUCT:
		return 1;
	default:
		break;
	}

	return 0;
}

/*
	���ʽ���
	<expression_statement>-><TK_SEMICOLON>|<expression><TK_SEMICOLON>
*/
void expression_statement()
{
	if (token != TK_SEMICILON)
	{
		expression();
		operand_pop();
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICILON);
}

/*
	<if_statement>-><KW_IF><TK_OPENPA><expression><TK_COLSEPA><statement>[<KW_ELSE><statement>]
*/
void if_statement(int *bsym, int *csym)
{
	int a, b;
	syntax_state = SNTX_SP;
	get_token();
	skip(TK_OPENPA);
	expression();
	syntax_state = SNTX_LF_HT;
	skip(TK_CLOSEPA);
	a = gen_jcc(0);
	statement(bsym, csym);
	if (token == KW_ELSE)
	{
		syntax_state = SNTX_LF_HT;
		get_token();
		b = gen_jmpforward(0);
		backpatch(a, ind);
		statement(bsym, csym);
		backpatch(b, ind);
	}
	else
		backpatch(a, ind);
}

/*
	<for_statement>-><KW_FOR><TK_OPENPA><expression_statement><expression_statement><expression><TK_CLOSEPA><statement>
*/
void for_statement(int *bsym, int *csym)
{
	int a, b, c, d, e;
	get_token();
	skip(TK_OPENPA);
	if (token != TK_SEMICILON)
	{
		expression();
		operand_pop();
	}
	skip(TK_SEMICILON);
	d = ind;
	c = ind;
	a = 0;
	b = 0;
	if (token != TK_SEMICILON)
	{
		expression();
		a = gen_jcc(0);
	}
	skip(TK_SEMICILON);
	if (token != TK_CLOSEPA)
	{
		e = gen_jmpforward(0);
		c = ind;
		expression();
		operand_pop();
		gen_jmpbackword(d);
		backpatch(e, ind);
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_CLOSEPA);
	statement(&a, &b);
	gen_jmpbackword(c);
	backpatch(a, ind);
	backpatch(b, c);
}

/*
	<continue_statement>-><KW_CONTINUE><TK_SIMICOLON>
*/
void continue_statement(int *csym)
{
	if (!csym)
		error("�˴�������continue");
	*csym = gen_jmpforward(*csym);
	get_token();
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICILON);
}

/*
	<break_statement>-><KW_BREAK><TK_SEMICOLON>
*/
void break_statement(int *bsym)
{
	if (!bsym)
		error("�˴�������break");
	*bsym = gen_jmpforward(*bsym);
	get_token();
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICILON);
}

/*
	<return_statement>-><KW_RETURN><TK_SEMICOLON>|<KW_RETURN><expression><TK_SEMICOLON>
*/
void return_statement()
{
	syntax_state = SNTX_DELAY;
	get_token();
	if (token == TK_SEMICILON)
		syntax_state = SNTX_NULL;
	else
		syntax_state = SNTX_SP;
	syntax_indent();

	if (token != TK_SEMICILON)
	{
		expression();
		load_1(REG_IRET, optop);
		operand_pop();
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICILON);
	rsym = gen_jmpforward(rsym);	//int *rsym
}

/*
	���ʽ
	<expression>-><assignment_expression>{<TK_COMMA><assignment_expression>}
*/
void expression()
{
	while (1)
	{
		assignment_expression();
		if (token != TK_COMMA)
			break;
		operand_pop();
		get_token();
	}
}

/*
	��ֵ���ʽ
	<assignment_expression>-><equality_expression>{<TK_ASSIGN><assignment_expression>}
*/
void assignment_expression()
{
	equality_expression();
	if (token == TK_ASSIN)
	{
		check_lvalue();
		get_token();
		assignment_expression();
		store0_1();
	}
}

/*
	�������ʽ
	<equality_expression>-><relation_expression>{<TK_EQ><relation_expression>
	|<TK_NEQ><relation_expression>}
*/
void equality_expression()
{
	int t;
	relation_expression();
	while (token==TK_EQ||token==TK_NEQ)
	{
		t = token;
		get_token();
		relation_expression();
		gen_op(t);
	}
}

/*
	��ϵ����ʽ
	<relation_expression>-><addtive_expression>{
	|<TK_LT><addtive_expression>
	|<TK_GT><addtive_expression>
	|<TK_LEQ><addtive_expression>
	|TK_GEQ><addtive_expression>}
*/
void relation_expression()
{
	int t;
	addtive_expression();
	while ((token==TK_LT||token==TK_LEQ)||token==TK_GT||token==TK_GEQ)
	{
		t = token;
		get_token();
		addtive_expression();
		gen_op(t);
	}
}

/*
	<addtive_expression>-><multuplicative_expresstion>
	|<TK_PLUS><multuplicative_expresstion>
	|<TK_MINUS><multuplicative_expresstion>
*/
void addtive_expression()
{
	int t;
	multuplicative_expresstion();
	while (token==TK_PLUS||token==TK_MINUS)
	{
		t = token;
		get_token();
		multuplicative_expresstion();
		gen_op(t);
	}
}

/*
	<multuplicative_expresstion>-><unary_expression>{<TK_STAR><unary_expression>|<TK_DIVIDE<unary_expression>|<TK_MOD><unary_expression>}
*/
void multuplicative_expresstion()
{
	int t;
	unary_expression();
	while (token==TK_STAR||token==TK_DIV||token==TK_MOD)
	{
		t = token;
		get_token();
		unary_expression();
		gen_op(t);
	}
}

/*
	һԪ���ʽ
	<unary_expression>-><postfix_expression>
	|<TK_AND><unary_expression>
	|<TK_STAR><unary_expression>
	|<TK_PLUS><unary_expression>
	|<TK_MINUS><unary_expression>
	|<sizeof_expression>
*/
void unary_expression()
{
	switch (token)
	{
	case TK_AND:
		get_token();
		unary_expression();
		if ((optop->type.t&T_BTYPE) != T_FUNC&&!(optop->type.t&T_ARRAY))
			cancel_lvalue();
		mk_pointer(&optop->type);
		break;
	case TK_STAR:
		get_token();
		unary_expression();
		indirection();
		break;
	case TK_PLUS:
		get_token();
		unary_expression();
		break;
	case TK_MINUS:
		get_token();
		operand_push(&int_type, BC_GLOBAL, 0);
		unary_expression();
		gen_op(TK_MINUS);
		break;
	case KW_SIZEOF:
		sizeof_expression();
		break;
	default:
		postfix_expression();
		break;
	}
}


/*
	<sizeof_expression>-><KW_SIZEOF><TK_OPENPA><type_spec><TK_CLOSEPA>
*/
void sizeof_expression()
{
	int align, size;
	Type type;

	get_token();
	skip(TK_OPENPA);
	type_spec(&type);
	skip(TK_CLOSEPA);

	size = type_size(&type, &align);
	if (size < 0)
		error("sizeof���ܼ������ͳߴ磡");
	operand_push(&int_type, BC_GLOBAL, size);
}

int type_size(Type *t, int *a)		//�������ͳ���
{
	Symbol *s;
	int bt;
	//ָ�����ͳ���Ϊ4�ֽ�
	int PTR_SIZE = 4;

	bt = t->t&T_BTYPE;
	switch (bt)
	{
	case T_STRUCT:
		s = t->ref;
		*a = s->r;
		return s->c;
	case T_PTR:
		if (t->t&T_ARRAY)
		{
			s = t->ref;
			return type_size(&s->type, a)*s->c;
		}
		else
		{
			*a = PTR_SIZE;
			return PTR_SIZE;
		}
	case T_SHORT:
		*a = 2;
		return 2;
	case T_INT:
		*a = 4;
		return 4;
	default:
		*a = 1;
		return 1;
		break;
	}
}
/*
	��׺���ʽ
	<postfix_expression>-><primary_expression>
	{<TK_OPENBR><expression><TK_CLOSEBR>
	|<TK_OPENPA><TK_CLOSEPA>
	|<TK_OPENPA><argument_expression_list><TK_COLSEPA>
	|<TK_DOT><IDENTIFIER>
	|<TK_POINTSTO><IDENTIFIER>}
*/
void postfix_expression()
{
	Symbol *s;
	primary_expression();
	while (1)
	{
		if (token == TK_DOT || token == TK_POINTSTO)
		{
			if (token == TK_POINTSTO)
				indirection();
			cancel_lvalue();
			get_token();
			if ((optop->type.t&T_BTYPE) != T_STRUCT)
				expect("�ṹ�����");
			s = optop->type.ref;
			token |= BC_MEMBER;
			while ((s = s->next) != NULL)
			{
				if (s->v == token)
					break;
			}
			if (!s)
				error("û�д˳�Ա����:%s", get_tkstr(token&~BC_MEMBER));
			/*	��Ա������ַ=�ṹ����ָ��+��Ա����ƫ��	*/
			optop->type = char_ptr_type;
			operand_push(&int_type, BC_GLOBAL, s->c);
			gen_op(TK_PLUS);
			/* �任����Ϊ��Ա������������ */
			optop->type = s->type;
			/* ���鲻�ܳ䵱��ֵ */
			if (!(optop->type.t&T_ARRAY))
			{
				optop->r |= BC_LVAL;
			}
			get_token();
		}
		else if (token == TK_OPENBR)
		{
			get_token();
			expression();
			gen_op(TK_PLUS);
			indirection();
			skip(TK_CLOSEBR);
		}
		else if (token == TK_OPENPA)
		{
			argument_expression_list();
		}
		else break;
	}
}

/*
	��ֵ���ʽ
	<primary_expression>-><IDENTIFER>|<TK_CINT>|<TK_CSTR>|<TK_CCHAR>|<TK_OPENPA><expression><TK_CLOSEPA>
*/
void primary_expression()
{
	int t, addr, r;
	Type type;
	Symbol *s;
	Section *sec = NULL;

	switch (token)
	{
	case TK_CINT:
	case TK_CCHAR:
		operand_push(&int_type, BC_GLOBAL, tkvalue);
		get_token();
		break;
	case TK_CSTR:
		t = T_CHAR;
		type.t = t;
		mk_pointer(&type);
		type.t |= T_ARRAY;
		sec = allocate_storage(&type, BC_GLOBAL, 2, 0, &addr);
		var_sym_put(&type, BC_GLOBAL, 0, addr);
		initializer(&type,addr,sec);
		break;

	case TK_OPENPA:
		get_token();
		expression();
		skip(TK_CLOSEPA);
		break;
	default:
		t = token;
		get_token();
		if (t < TK_IDENTI)
			expect("��ʶ������");
		s = sym_search(t);
		if (!s)
		{
			if (token != TK_OPENPA)
				error("'%s'δ����\n", get_tkstr(t));
			s = func_sym_push(t, &default_func_type);
			s->r = BC_GLOBAL | BC_SYM;
		}
		r = s->r;
		operand_push(&s->type, r, s->c);
		/*	�������ã������������¼���ŵ�ַ	*/
		if (optop->r&BC_SYM)
		{
			optop->sym = s;
			optop->value = 0;
		}
		break;
	}
}

/*
	ʵ�α��ʽ
	<argument_expression_list>-><assignment_expression>
	{<TK_COMMA><assignment_expression>}
*/
void argument_expression_list()
{
	Operand ret;
	Symbol *s, *sa;
	int nb_args;
	s = optop->type.ref;
	get_token();
	sa = s->next;
	nb_args = 0;
	ret.type = s->type;
	ret.r = REG_IRET;
	ret.value = 0;
	if (token!=TK_CLOSEPA)
	{
		for (;;)
		{
			assignment_expression();
			nb_args++;
			if (sa)
				sa = sa->next;
			if (token == TK_CLOSEPA)
				break;
			skip(TK_COMMA);
		}
	}
	if (sa)
		error("ʵ�θ��������βθ���");
	skip(TK_CLOSEPA);
	gen_invoke(nb_args);
	operand_push(&ret.type, ret.r, ret.value);
}

/*
	�﷨����
*/
void syntax_indent()
{
	switch (syntax_state)
	{
	case SNTX_NULL:
		color_token(LEX_NORMAL);
		break;
	case SNTX_SP:
		printf(" ");
		color_token(LEX_NORMAL);
		break;
	case SNTX_LF_HT:
	{
					   if (token == TK_END)
						   syntax_level--;
					   printf("\n");
					   print_tab(syntax_level);
	}
		color_token(LEX_NORMAL);
		break;
	case SNTX_DELAY:
		break;
	}
	syntax_state = SNTX_NULL;
}

void print_tab(int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("\t");
	}
}
