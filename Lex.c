/*
	Lex.cpp
	词法分析主程序
*/

#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include"BCC.h"
#include"Lex.h"
#include"Parsing.h"
#include"SymbolTable.h"
#include"Stack.h"

TkWord* tk_hashtable[MAXKEY];		//单词哈希表
DynArray tktable;					//单词表放置标识符
DynString tkstr;					//单词字符串
DynString sourcestr;				//单词源字符串
int token;							//单词编码
char ch;							//当前源码字符
int lineno;							//行号
int tkvalue;						//单词值
char *filename;				//文件名


extern Stack global_sym_stack;	//全局符号表栈
extern Stack local_sym_stack;		//局部符号表栈
extern FILE* fin;							//文件指针

void init_lex()						//词法分析初始化
{
	TkWord* tp;
	static TkWord keywords[] = {
		{ TK_PLUS, NULL, "+", NULL, NULL },
		{ TK_MINUS, NULL, "-", NULL, NULL },
		{ TK_STAR, NULL, "*", NULL, NULL },
		{ TK_DIV, NULL, "/", NULL, NULL },
		{ TK_MOD, NULL, "%", NULL, NULL },
		{ TK_EQ, NULL, "==", NULL, NULL },
		{ TK_NEQ, NULL, "!=", NULL, NULL },
		{ TK_LT, NULL, "<", NULL, NULL },
		{ TK_LEQ, NULL, "<=", NULL, NULL },
		{ TK_GT, NULL, ">", NULL, NULL },
		{ TK_GEQ, NULL, ">=", NULL, NULL },
		{ TK_ASSIN, NULL, "=", NULL, NULL },
		{ TK_POINTSTO, NULL, "->", NULL, NULL },
		{ TK_DOT, NULL, ".", NULL, NULL },
		{ TK_AND, NULL, "&", NULL, NULL },
		{ TK_OPENPA, NULL, "(", NULL, NULL },
		{ TK_CLOSEPA, NULL, ")", NULL, NULL },
		{ TK_OPENBR, NULL, "[", NULL, NULL },
		{ TK_CLOSEBR, NULL, "]", NULL, NULL },
		{ TK_BEGIN, NULL, "{", NULL, NULL },
		{ TK_END, NULL, "}", NULL, NULL },
		{ TK_SEMICILON, NULL, ";", NULL, NULL },
		{ TK_COMMA, NULL, ",", NULL, NULL },
		{ TK_ELLPI, NULL, "...", NULL, NULL },
		{ TK_EOF, NULL, "End_of_File", NULL, NULL },

		{ TK_CINT, NULL, "整型常量", NULL, NULL },
		{ TK_CCHAR, NULL, "字符常量", NULL, NULL },
		{ TK_CSTR, NULL, "字符串常量", NULL, NULL },

		{ KW_CHAR, NULL, "char", NULL, NULL },
		{ KW_SHORT, NULL, "short", NULL, NULL },
		{ KW_INT, NULL, "int", NULL, NULL },
		{ KW_VOID, NULL, "void", NULL, NULL },
		{ KW_STRUCT, NULL, "struct", NULL, NULL },

		{ KW_IF, NULL, "if", NULL, NULL },
		{ KW_ELSE, NULL, "else", NULL, NULL },
		{ KW_FOR, NULL, "for", NULL, NULL },
		{ KW_CONTINUE, NULL, "continue", NULL, NULL },
		{ KW_BREAK, NULL, "break", NULL, NULL },
		{ KW_RETURN, NULL, "return", NULL, NULL },
		{ KW_SIZEOF, NULL, "sizeof", NULL, NULL },
		{ KW_ALIGN, NULL, "__align", NULL, NULL },
		{ KW_CDECL, NULL, "__cdecl", NULL, NULL },
		{ KW_STDCALL, NULL, "__stdcall", NULL, NULL },

		{ 0, NULL, NULL, NULL, NULL },
	};
	dynarray_init(&tktable, 8);
	for (tp = &keywords[0]; tp->str != NULL; tp++)
	{
		tkWord_direct_insert(tp);
	}
}

void getch()			//从源文件中读取一个字符
{
	ch = getc(fin);
}

void skip_space()		//跳过空白字符
{
	while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
	{
		if (ch == '\r')
		{
			getch();
			if (ch != '\n')
				return;
			lineno++;
		}
		else if (ch == '\n')
			lineno++;
		printf("%c", ch);
		getch();
	}
}

void lexical_comment()		//处理注释
{
	getch();
	for (;;)
	{
		do
		{
			if (ch == '\n' || ch == '*' || ch == EOF)
				break;
			else 
				getch();
		} while (1);
		if (ch == '\n')
		{
			lineno++;
			getch();
		}
		else if (ch == '*')
		{
			getch();
			if (ch == '/')
			{
				getch();
				return;
			}
		}
		else
		{
			error("末尾才有注释结束符");
			return;
		}
	}
}

void preprocess()			//预处理，忽略空白及注释
{
	for (;;)
	{
		if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
		{
			skip_space();
		}
		else if (ch == '/')
		{
			getch();
			if (ch == '*')
			{
				lexical_comment();
			}
			else
			{
				ungetc(ch, fin);
				ch = '/';
				break;
			}
		}
		else break;
	}
}

int is_letter(char c)			//是否是字母下划线
{
	return (c >= 'a'&&c <= 'z') || (c >= 'A'&&c <= 'Z') || c == '_';
}

int is_digit(char c)			//是否是数字
{
	return c >= '0'&&c <= '9';
}

void lexical_identifier()		//标识符的处理
{
	dynstring_reset(&tkstr);
	dynstring_chcat(&tkstr, ch);
	getch();
	while (is_letter(ch)||is_digit(ch))
	{
		dynstring_chcat(&tkstr, ch);
		getch();
	}
	dynstring_chcat(&tkstr, '\0');
}

void lexical_num()				//解析整型常量
{
	dynstring_reset(&tkstr);
	dynstring_reset(&sourcestr);
	do
	{
		dynstring_chcat(&tkstr, ch);
		dynstring_chcat(&sourcestr, ch);
		getch();
	} while (is_digit(ch));
	if (ch == '.')
	{
		do
		{
			dynstring_chcat(&tkstr, ch);
			dynstring_chcat(&sourcestr, ch);
			getch();
		} while (is_digit(ch));
	}
	dynstring_chcat(&tkstr, '\0');
	dynstring_chcat(&sourcestr, '\0');
	tkvalue = atoi(tkstr.data);
}

void lexical_string(char s)					//解析字符串
{
	char c;
	dynstring_reset(&tkstr);
	dynstring_reset(&sourcestr);
	dynstring_chcat(&sourcestr, s);
	getch();
	for (;;)
	{
		if (ch == s)
			break;
		else if (ch == '\\')
		{
			dynstring_chcat(&sourcestr, ch);
			getch();
			switch (ch)							//转义字符
			{
			case '0':
				c = 0; break;
			case 'a':
				c = '\a'; break;
			case 'b':
				c = '\b'; break;
			case 't':
				c = '\t'; break;
			case 'n':
				c = '\n'; break;
			case 'v':
				c = '\v'; break;
			case 'f':
				c = '\f'; break;
			case 'r':
				c = '\r'; break;
			case '\"':
				c = '\"'; break;
			case '\'':
				c = '\''; break;
			case '\\':
				c = '\\'; break;
			default:
				c = ch;
				if (c >= '!'&&c <= '~')
					warning("非法字符：\'\\%c'", c);
				else
					warning("非法转义字符：\'\\0x%x\'", c);
				break;
			}
			dynstring_chcat(&tkstr, c);
			dynstring_chcat(&sourcestr, ch);
			getch();
		}
		else
		{
			dynstring_chcat(&tkstr, ch);
			dynstring_chcat(&sourcestr, ch);
			getch();
		}
	}
	dynstring_chcat(&tkstr, '\0');
	dynstring_chcat(&sourcestr, s);
	dynstring_chcat(&sourcestr, '\0');
	getch();
}

void get_token()			//取单词
{
	preprocess();
	if ((ch >= 'a'&&ch <= 'z') || (ch >= 'A'&&ch <= 'Z') || (ch == '_'))
	{
		TkWord *tp;
		lexical_identifier();
		tp = tkword_insert(tkstr.data);
		token = tp->tkcode;
	}
	else if (ch >= '0'&&ch <= '9')
	{
		lexical_num();
		token = TK_CINT;
	}
	else if (ch == '+')
	{
		getch();
		token = TK_PLUS;
	}
	else if (ch == '-')
	{
		getch();
		if (ch == '>')
		{
			token = TK_POINTSTO;
			getch();
		}
		token = TK_MINUS;
	}
	else if (ch == '/')
	{
		token = TK_DIV;
		getch();
	}
	else if (ch == '%')
	{
		token = TK_MOD;
		getch();
	}
	else if (ch == '=')
	{
		getch();
		if (ch == '=')
		{
			token = TK_EQ;
			getch();
		}
		else
			token = TK_ASSIN;
	}
	else if (ch == '!')
	{
		getch();
		if (ch == '=')
		{
			token = TK_NEQ;
			getch();
		}
		else
			error("非法操作符！");
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			token = TK_LEQ;
			getch();
		}
		else
			token = TK_LT;
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			token = TK_GEQ;
			getch();
		}
		else
			token = TK_GT;
	}
	else if (ch == '.')
	{
		getch();
		if (ch == '.')
		{
			getch();
			if (ch != '.')
				error("省略号拼写有误！");
			else
				token = TK_ELLPI;
			getch();
		}
		else
		{
			token = TK_DOT;
		}
	}
	else if (ch == '&')
	{
		token = TK_AND;
		getch();
	}
	else if (ch == ';')
	{
		token = TK_SEMICILON;
		getch();
	}
	else if (ch == ']')
	{
		token = TK_CLOSEBR;
		getch();
	}
	else if (ch == '}')
	{
		token = TK_END;
		getch();
	}
	else if (ch == ')')
	{
		token = TK_CLOSEPA;
		getch();
	}
	else if (ch == '[')
	{
		token = TK_OPENBR;
		getch();
	}
	else if (ch == '{')
	{
		token = TK_BEGIN;
		getch();
	}
	else if (ch == '(')
	{
		token = TK_OPENPA;
		getch();
	}
	else if (ch == ',')
	{
		token = TK_COMMA;
		getch();
	}
	else if (ch == '*')
	{
		token = TK_STAR;
		getch();
	}
	else if (ch == '\'')
	{
		lexical_string(ch);
		token = TK_CCHAR;
		tkvalue = *(char*)tkstr.data;
	}
	else if (ch == '\"')
	{
		lexical_string(ch);
		token = TK_CSTR;
	}
	else if (ch == EOF)
	{
		token = TK_EOF;
	}
	else
	{
		error("非法的字符：%c",ch);
		getch();
	}
	syntax_indent();
}

void color_token(int lex_state)		//单词着色
{
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	char* p;
	switch (lex_state)
	{
	case LEX_NORMAL:
	{
		 if (token >= TK_IDENTI)
			   SetConsoleTextAttribute(h, FOREGROUND_INTENSITY);
	     else if (token >= KW_CHAR)
			   SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY );
	     else if (token>=TK_CINT)
			   SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN);
		 else
			   SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);
			   p = get_tkstr(token);
			   printf("%s", p);
			   break;
	}
	case LEX_SEP:
		printf("%c", ch);
		break;
	}
}



//int main()				//词法分析测试main函数
//{
//	printf("请输入所做词法分析的文件名!\n");
//	//gets(filename);
//	scanf("%s", filename);
//	fin = fopen(filename,"r");
//	if (!fin)
//	{
//		printf("不能打开文件！\n");
//		system("pause");
//		return 0;
//	}
//
//	init();
//	getch();
//	get_token();
//
//	parsing();
//	//do{
//	//	get_token();
//	//	color_token(LEX_NORMAL);
//	//} while (token != TK_EOF);
//
//	printf("\n%s语法分析结束！", filename);
//	printf("\n代码行数 = %d 行\n", lineno);
//	cleanup();
//	fclose(fin);
//	system("pause");
//	return 1;
//}
