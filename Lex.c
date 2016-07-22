/*
	Lex.cpp
	�ʷ�����������
*/

#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include"BCC.h"
#include"Lex.h"
#include"Parsing.h"
#include"SymbolTable.h"
#include"Stack.h"

TkWord* tk_hashtable[MAXKEY];		//���ʹ�ϣ��
DynArray tktable;					//���ʱ���ñ�ʶ��
DynString tkstr;					//�����ַ���
DynString sourcestr;				//����Դ�ַ���
int token;							//���ʱ���
char ch;							//��ǰԴ���ַ�
int lineno;							//�к�
int tkvalue;						//����ֵ
char *filename;				//�ļ���


extern Stack global_sym_stack;	//ȫ�ַ��ű�ջ
extern Stack local_sym_stack;		//�ֲ����ű�ջ
extern FILE* fin;							//�ļ�ָ��

void init_lex()						//�ʷ�������ʼ��
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

		{ TK_CINT, NULL, "���ͳ���", NULL, NULL },
		{ TK_CCHAR, NULL, "�ַ�����", NULL, NULL },
		{ TK_CSTR, NULL, "�ַ�������", NULL, NULL },

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

void getch()			//��Դ�ļ��ж�ȡһ���ַ�
{
	ch = getc(fin);
}

void skip_space()		//�����հ��ַ�
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

void lexical_comment()		//����ע��
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
			error("ĩβ����ע�ͽ�����");
			return;
		}
	}
}

void preprocess()			//Ԥ�������Կհ׼�ע��
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

int is_letter(char c)			//�Ƿ�����ĸ�»���
{
	return (c >= 'a'&&c <= 'z') || (c >= 'A'&&c <= 'Z') || c == '_';
}

int is_digit(char c)			//�Ƿ�������
{
	return c >= '0'&&c <= '9';
}

void lexical_identifier()		//��ʶ���Ĵ���
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

void lexical_num()				//�������ͳ���
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

void lexical_string(char s)					//�����ַ���
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
			switch (ch)							//ת���ַ�
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
					warning("�Ƿ��ַ���\'\\%c'", c);
				else
					warning("�Ƿ�ת���ַ���\'\\0x%x\'", c);
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

void get_token()			//ȡ����
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
			error("�Ƿ���������");
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
				error("ʡ�Ժ�ƴд����");
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
		error("�Ƿ����ַ���%c",ch);
		getch();
	}
	syntax_indent();
}

void color_token(int lex_state)		//������ɫ
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



//int main()				//�ʷ���������main����
//{
//	printf("�����������ʷ��������ļ���!\n");
//	//gets(filename);
//	scanf("%s", filename);
//	fin = fopen(filename,"r");
//	if (!fin)
//	{
//		printf("���ܴ��ļ���\n");
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
//	printf("\n%s�﷨����������", filename);
//	printf("\n�������� = %d ��\n", lineno);
//	cleanup();
//	fclose(fin);
//	system("pause");
//	return 1;
//}
