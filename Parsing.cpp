/*
	Parsing.cpp
	语法分析程序
*/
#include<stdio.h>
#include<stdlib.h>
#include"Lex.h"
#include"BCC.h"
#include"Parsing.h"
#include"SymbolTable.h"

int syntax_state;	//语法状态
int syntax_level;	//缩紧级别

void parsing()				//语法分析单元
{
	while (token != TK_EOF)
	{
		external_decl(BC_GLOBAL);
	}
}

/***********************************************************************
	外部声明处理
	文法为：
	<external_decl>-><type_spec>(<TK_SEMICOLON>|<declarator><function_body>|
	<declarator>[<TK_ASSIGN><initializer>]
	{<TK_COMMA><declarator>[TK_ASSIGN<initializer>]}<TK_SEMI>)
***********************************************************************/

void external_decl(int b)	//解析外部声明		
{
	//Type btype, type;
	//int v, has_init, r, addr;
	//Symbol * sym;
	if (!type_spec())
	{
		expect("<类型区分符>");
	}
	if (token == TK_SEMICILON)
	{
		get_token();
		return;
	}
	while (b)
	{
		//type = btype;
		declarator();
		if (token == TK_BEGIN)		//函数定义
		{
			if (b == BC_LOCAL)
				error("不支持函数嵌套定义");
			funcbody();
			break;
		}
		else
		{
			if (token == TK_ASSIN)
			{
				get_token();
				initializer();
			}
			if (token == TK_COMMA)
				get_token();
			else
			{
				syntax_state = SNTX_LF_HT;
						skip(TK_SEMICILON);
						break;
			}
		}
		/*	if ((type.t&T_BTYPE) != T_FUNC)
				expect("<函数定义>");
		*/
		//	sym = sym_search(v);
		//	if (sym)
		//	{
		//		if ((sym->type.t&T_BTYPE) != T_FUNC)
		//			error("'%s'重定义", get_tkstr(v));
		//		sym->type = type;
		//	}
		//	else
		//	{
		//		sym = func_sym_push(v, &type);
		//	}
		//	sym->r = BC_SYM | BC_GLOBAL;

		//	funcbody(sym);
		//	break;
		//}
		//else
		//{
		////	if ((type.t&T_BTYPE) == T_FUNC)		//函数声明
		////	{
		////		if (sym_search(v) == NULL)
		////		{
		////			sym = sym_push(v, &type, BC_GLOBAL | BC_SYM, 0);
		////		}
		////	}
		////	else
		////	{
		////		r = 0;
		////		if (!(type.t&T_ARRAY))
		////			r |= BC_LVAL;
		////		r |= 1;
		////		has_init=(token == TK_ASSIN);

		//		if (has_init)
		//		{
		//			get_token();
		//			initializer();
		//		}
		//		sym = var_sym_put(&type, r, v, addr);
		//	}
		//	if (token == TK_COMMA)
		//	{
		//		get_token();
		//	}
		//	else
		//	{
		//		syntax_state = SNTX_LF_HT;
		//		skip(TK_SEMICILON);
		//		break;
		//	}
		//}
	}
}

int type_spec()				//类型区分符
{
	/*int t;
	Type type1;*/
	int type_found = 0;
	switch (token)
	{
	case KW_CHAR:
		//t = T_CHAR;
		syntax_state = SNTX_SP;
		type_found = 1;
		get_token();
		break;
	case KW_SHORT:
	//	t = T_SHORT;
		syntax_state = SNTX_SP;
		type_found = 1;
		get_token();
		break;
	case KW_VOID:
	//	t = T_VOID;
		syntax_state = SNTX_SP;
		type_found = 1;
		get_token();
		break;
	case KW_INT:
	//	t = T_INT;
		syntax_state = SNTX_SP;
		type_found = 1;
		get_token();
		break;
	case KW_STRUCT:
		syntax_state = SNTX_SP;
		struct_spec();
	//	type->ref = type1.ref;
	//	t = T_STRUCT;
		type_found = 1;
		break;
	default:
		break;
	}
//	type->t = t;
	return type_found;
}

/*
	区分结构符
	<struct_spec>-><KW_STRUCT><IDENTIFIER><TK_BEGIN><struct_decl_list><TK_END>
	|<KW_STRUCT><INDENTIFIER>
*/
void struct_spec()
{
	int v;
	//Symbol *s;
	//Type type1;

	get_token();
	v = token;

	syntax_state = SNTX_DELAY;
	get_token();

	if (token == TK_BEGIN)				//结构体定义的缩进
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA)
		syntax_state = SNTX_NULL;
	else
		syntax_state = SNTX_SP;
	syntax_indent();

	if (v < TK_IDENTI)			//关键字不能作为结构体名
		expect("结构体名");

	if (token == TK_BEGIN)
	{
		struct_decl_list();
	}

	//s = struct_search(v);
	//if (!s)
	//{
	//	type1.t = KW_STRUCT;
	//	s = sym_push(v | BC_STRUCT, &type1, 0, -1);
	//	s->r = 0;
	//}

	//type->t = T_STRUCT;
	//type->ref = s;
}

/*
	解析结构声明符表
	<struct_decl_list>-><struct_decl>{<struct_decl>}
*/
void struct_decl_list()
{
	int maxalign, offset;
	//Symbol *s, **ps;
	//s = type->ref;

	syntax_state = SNTX_LF_HT;		//结构体成员缩进
	syntax_level++;

	get_token();
	//if (s->c != -1)
	//	error("结构体已定义");
	//maxalign = 1;
	//ps = &s->next;
	//offset = 0;

	while (token!=TK_END)
	{
		struct_decl(/*&maxalign, &offset,&ps*/);
	}
	skip(TK_END);

	syntax_state = SNTX_LF_HT;
	//s->c = calc_align(offset, maxalign);
	//s->r = maxalign;
}

int calc_align(int n, int align)		//计算字节对齐位置
{
	return ((n + align - 1)&(~(align - 1)));
}

/*
	<struct_decl>-><type_spec><declarator>{<TK_COMMA><declaration>}<TK_SIMICOLON>
*/
void struct_decl(/*int *maxalign, int *offset, Symbol ***ps*/)
{
	//int v, size, align;
	//Symbol *ss;
	//Type type1, btype;
	//int force_align;
	type_spec();
	while (1)
	{
	/*	v = 0;
		type1 = btype;*/
		declarator(/*&type1, &v, &force_align*/);
	/*	size = type_size(&type1, &align);
		if (force_align&ALIGN_SET)
			align = force_align&~ALIGN_SET;

		*offset = calc_align(*offset, align);

		if (align > *maxalign)
			*maxalign = align;
		ss = sym_push(v | BC_MEMBER, &type1, 0, *offset);
		*offset += size;
		**ps = ss;
		*ps = &ss->next;*/

		if (token == TK_SEMICILON)
			break;
		skip(TK_COMMA);
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICILON);
}

/*
	预留：函数调用约定
	<function_call_convention>-><KW_CDECL>|<KW_STDCALL>
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
	预留：结构体成员对齐
	<struct_member_alignment>
*/
void struct_mem_align(/*int *force_align*/)
{
	//int align = 1;
	if (token == KW_ALIGN)
	{
		get_token();
		skip(TK_OPENPA);
		if (token == TK_CINT)
		{
			get_token();
			//align = tkvalue;
		}
		else
			expect("整数常量");
		skip(TK_CLOSEPA);
		/*if (align != 1 && align != 2 && align != 4)
			align = 1;
		align |= ALIGN_SET;
		*force_align = align;*/
	}
	/*else
		*force_align;*/
}

/*
	声明符
	<declarator>->{<TK_STAR>}[<function_call_convention>][<struct_member_alignment>]<direct_declarator>
*/
void declarator(/*Type *type,int *v,int *force_align*/)
{
	int fc;
	while (token==TK_STAR)
	{
		//mk_pointer(type);
		get_token();
	}
	function_call_convention(&fc);
	//if (!force_align);
	struct_mem_align(/*force_align*/);
	direct_declarator(/*type, v, */fc);
}

/*
	直接声明符
	<direct_declarator>-><IDENTIFIER><direct_declarator_postfix>
*/
void direct_declarator(/*Type *type,int *v,*/ int func_call)
{
	if (token >= TK_IDENTI)
	{
		//*v = token;
		get_token();
	}
	else
	{
		expect("标识符");
	}
	direct_declarator_postfix(/*type,*/func_call);
}

/*
	直接声明后缀符
	<direct_declarator_postfix>->{<TK_OPENBR><TK_CINT><TK_CLOSEBR>
	|<TK_OPENBR><TK_CLOSEBR>
	|<TK_OPENPA><parameter_type_list><TK_CLOSEPA>
	|<TK_OPENPA><TK_CLOSEPA>}
*/
void direct_declarator_postfix(/*Type *type,*/int func_call)
{
	int n;
	//Symbol *s;

	if (token == TK_OPENPA)
	{
		parameter_type_list(/*type,*/func_call);
	}
	else if (token == TK_OPENBR)
	{
		get_token();
		//n = -1;
		if (token = TK_CINT)
		{
			get_token(); 
			n = tkvalue;
		}
		skip(TK_CLOSEBR);
		direct_declarator_postfix(/*type,*/func_call);
	/*	s = sym_push(BC_ANOM, type, 0, n);
		type->t = T_ARRAY | T_PTR;
		type->ref;*/
	}
}

/*
	形参类型表
	<parameter_type_list>-><type_spec>{<declarator>}
	{<TK_COMMA><type_spec>{<declarator>}}<TK_COMMA><TK_ELLIPSIS>
*/
void parameter_type_list(/*Type *type, */int func_call)
{
	//int n;
	//Symbol **plast, *s, *first;
	//Type pt;

	get_token();
	/*first = NULL;
	plast = &first;
*/
	while (token != TK_CLOSEPA)
	{
		if (!type_spec())
			error("无效类型符！");
		declarator();
		//s = sym_push(n|BC_PARAMS, &pt, 0,0);
	//	*plast = s;
	//	plast = &s->next;
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

	//s = sym_push(BC_ANOM, type, func_call, 0);		//将函数返回类型存储，然后指向参数最后将type设为函数类型，引用的相关信息
	//s->next = first;								//
	//type->t = T_FUNC;
	//type->ref = s;

	syntax_state = SNTX_DELAY;
	skip(TK_CLOSEPA);
	if (token == TK_BEGIN)			//函数定义
		syntax_state = SNTX_LF_HT;
	else                           //函数声明
		syntax_state = SNTX_NULL;
	syntax_indent();
}

/*
	函数体
	<funcbody>-><compound_statement>
*/
void funcbody(/*Symbol *sym*/)
{
	/* 放一匿名符号在局部符号表中 */
	//sym_direct_push(&local_sym_stack, BC_ANOM, &int_type, 0);
	compound_statement();

	//sym_pop(&local_sym_stack, NULL);

}

/*
	初值符
	<initializer>-><assignment_expression>
*/
void initializer(/*Type *type*/)
{
	//if (type->t&T_ARRAY)
	//{
	//	get_token();
	//}
	//else
		assignment_expression();
}

/*
	语句
	<statement>-><compound_statement>|<if_statement>|<return_statement>|<break_statement>|<continue_statement>
	|<for_statement>|<expression_statement>
*/
void statement(/*int *bsym,int *csym*/)
{
	switch (token)
	{
	case TK_BEGIN:
		compound_statement();
		break;
	case KW_IF:
		if_statement();
		break;
	case KW_RETURN:
		return_statement();
		break;
	case KW_BREAK:
		break_statement();
		break;
	case KW_CONTINUE:
		continue_statement();
		break;
	case KW_FOR:
		for_statement();
		break;
	default:
		expression_statement();
		break;
	}
}

/*
	复合语句
	<compound_statement>-><TK_BEGIN>{<declaration>}{<statement>}<TK_END>
	PS：事实上<declaration>和<statement>的顺序可以交换
	changed1
*/
void compound_statement(/*int *bsym,int *csym*/)
{
	syntax_state = SNTX_LF_HT;			//复合语句
	syntax_level++;

	//Symbol *s;
	//s = (Symbol*)get_top(&local_sym_stack);

	get_token();
	//while (is_type_spec(token))
	//{
	//	external_decl(BC_LOCAL);
	//}
	while (token!=TK_END)
	{
		while (is_type_spec(token))
		{
			external_decl(BC_LOCAL);
		}
		statement();
	}
	syntax_state = SNTX_LF_HT;
	//sym_pop(&local_sym_stack,s);
	get_token();
}

/*
	判断是否为类型区分符
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
	表达式语句
	<expression_statement>-><TK_SEMICOLON>|<expression><TK_SEMICOLON>
*/
void expression_statement()
{
	if (token != TK_SEMICILON)
	{
		expression();
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICILON);
}

/*
	<if_statement>-><KW_IF><TK_OPENPA><expression><TK_COLSEPA><statement>[<KW_ELSE><statement>]
*/
void if_statement()
{
	syntax_state = SNTX_SP;
	get_token();
	skip(TK_OPENPA);
	expression();
	syntax_state = SNTX_LF_HT;
	skip(TK_CLOSEPA);
	statement();
	if (token == KW_ELSE)
	{
		syntax_state = SNTX_LF_HT;
		get_token();
		statement();
	}
}

/*
	<for_statement>-><KW_FOR><TK_OPENPA><expression_statement><expression_statement><expression><TK_CLOSEPA><statement>
*/
void for_statement()
{
	get_token();
	skip(TK_OPENPA);
	if (token != TK_SEMICILON)
	{
		expression();
	}
	skip(TK_SEMICILON);
	if (token != TK_SEMICILON)
	{
		expression();
	}
	skip(TK_SEMICILON);
	if (token != TK_CLOSEPA)
	{
		expression();
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_CLOSEPA);
	statement();
}

/*
	<continue_statement>-><KW_CONTINUE><TK_SIMICOLON>
*/
void continue_statement()
{
	get_token();
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICILON);
}

/*
	<break_statement>-><KW_BREAK><TK_SEMICOLON>
*/
void break_statement()
{
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
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICILON);
}

/*
	表达式
	<expression>-><assignment_expression>{<TK_COMMA><assignment_expression>}
*/
void expression()
{
	while (true)
	{
		assignment_expression();
		if (token != TK_COMMA)
			break;
		get_token();
	}
}

/*
	赋值表达式
	<assignment_expression>-><equality_expression>{<TK_ASSIGN><assignment_expression>}
*/
void assignment_expression()
{
	equality_expression();
	if (token == TK_ASSIN)
	{
		get_token();
		assignment_expression();
	}
}

/*
	相等类表达式
	<equality_expression>-><relation_expression>{<TK_EQ><relation_expression>
	|<TK_NEQ><relation_expression>}
*/
void equality_expression()
{
	relation_expression();
	while (token==TK_EQ||token==TK_NEQ)
	{
		get_token();
		relation_expression();
	}
}

/*
	关系类表达式
	<relation_expression>-><addtive_expression>{
	|<TK_LT><addtive_expression>
	|<TK_GT><addtive_expression>
	|<TK_LEQ><addtive_expression>
	|TK_GEQ><addtive_expression>}
*/
void relation_expression()
{
	addtive_expression();
	while ((token==TK_LT||token==TK_LEQ)||token==TK_GT||token==TK_GEQ)
	{
		get_token();
		addtive_expression();
	}
}

/*
	<addtive_expression>-><multuplicative_expresstion>
	|<TK_PLUS><multuplicative_expresstion>
	|<TK_MINUS><multuplicative_expresstion>
*/
void addtive_expression()
{
	multuplicative_expresstion();
	while (token==TK_PLUS||token==TK_MINUS)
	{
		get_token();
		multuplicative_expresstion();
	}
}

/*
	<multuplicative_expresstion>-><unary_expression>{<TK_STAR><unary_expression>|<TK_DIVIDE<unary_expression>|<TK_MOD><unary_expression>}
*/
void multuplicative_expresstion()
{
	unary_expression();
	while (token==TK_STAR||token==TK_DIV||token==TK_MOD)
	{
		get_token();
		unary_expression();
	}
}

/*
	一元表达式
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
		break;
	case TK_STAR:
		get_token();
		unary_expression();
		break;
	case TK_PLUS:
		get_token();
		unary_expression();
		break;
	case TK_MINUS:
		get_token();
		unary_expression();
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
	//int align, size;
	//Type type;

	get_token();
	skip(TK_OPENPA);
	type_spec();
	skip(TK_CLOSEPA);

//	size = type_size(&type, &align);
	/*if (size < 0)
		error("sizeof不能计算改类型尺寸！");*/
}

int type_size(Type *t, int *a)		//返回类型长度
{
	Symbol *s;
	int bt;
	//指针类型长度为4字节
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
	后缀表达式
	<postfix_expression>-><primary_expression>
	{<TK_OPENBR><expression><TK_CLOSEBR>
	|<TK_OPENPA><TK_CLOSEPA>
	|<TK_OPENPA><argument_expression_list><TK_COLSEPA>
	|<TK_DOT><IDENTIFIER>
	|<TK_POINTSTO><IDENTIFIER>}
*/
void postfix_expression()
{
	primary_expression();
	while (true)
	{
		if (token == TK_DOT || token == TK_POINTSTO)
		{
			get_token();
			token |= BC_MEMBER;
			get_token();
		}
		else if (token == TK_OPENBR)
		{
			get_token();
			expression();
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
	初值表达式
	<primary_expression>-><IDENTIFER>|<TK_CINT>|<TK_CSTR>|<TK_CCHAR>|<TK_OPENPA><expression><TK_CLOSEPA>
*/
void primary_expression()
{
	int t;
	/*Type type;
	Symbol *s;
*/
	switch (token)
	{
	case TK_CINT:
	case TK_CCHAR:
		get_token();
		break;
	case TK_CSTR:
		get_token();
		break;
	/*	t = T_CHAR;
		type.t = t;
		mk_pointer(&type);
		type.t |= T_ARRAY;
		var_sym_put(&type, BC_GLOBAL, 0, addr);
		initializer(&type);
		break;
*/
	case TK_OPENPA:
		get_token();
		expression();
		skip(TK_CLOSEPA);
		break;
	default:
		t = token;
		get_token();
		if (t < TK_IDENTI)
			expect("标识符或常量");
		/*s = sym_search(t);
		if (!s)
		{
			if (token != TK_OPENPA)
				error("'%s'未声明\n", get_tkstr(t));
			s = func_sym_push(t, &default_func_type);
			s->r = BC_GLOBAL | BC_SYM;
		}*/
		break;
	}
}

/*
	实参表达式
	<argument_expression_list>-><assignment_expression>
	{<TK_COMMA><assignment_expression>}
*/
void argument_expression_list()
{
	get_token();
	if (token!=TK_CLOSEPA)
	{
		for (;;)
		{
			assignment_expression();
			if (token == TK_CLOSEPA)
				break;
			skip(TK_COMMA);
		}
	}
	skip(TK_CLOSEPA);
}

/*
	语法缩进
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
