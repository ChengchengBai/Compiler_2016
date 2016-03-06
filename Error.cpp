/*
		Error.cpp
		�������쳣����
*/

#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include"Lex.h"
#include"BCC.h"

void warning(char* fmt, ...)		//���뾯��
{
	va_list ap;
	va_start(ap, fmt);

	char buf[1024];
	vsprintf(buf, fmt, ap);

	printf("\n%s(��%d��)�����뾯�棺%s!\n", filename, lineno, buf);

	va_end(ap);
}

void error(char* fmt, ...)			//���������
{
	va_list ap;
	va_start(ap, fmt);

	char buf[1024];
	vsprintf(buf, fmt, ap);

	printf("\n%s(��%d��)���������%s!\n", filename, lineno, buf);
	system("pause");
	exit(-1);

	va_end(ap);
}

void link_error(char* fmt, ...)		//���Ӵ���
{
	va_list ap;
	va_start(ap, fmt);

	char buf[1024];
	vsprintf(buf, fmt, ap);

	printf("\t���Ӵ���%s��\n", buf);
	system("pause");
	exit(-1);

}

void expect(char* msg)				//ȱ��ĳ���﷨�ɷ�
{
	error("\tȱ��%s", msg);
}

void skip(int c)					//������ǰ����
{
	if (token != c)
		error("\tȱ��'%s'", get_tkstr(c));
	get_token();
}

char* get_tkstr(int c)
{
	if (c > tktable.count)
		return NULL;
	else if (c >= TK_CINT&&c <= TK_CSTR)
		return sourcestr.data;
	else
		return ((TkWord*)tktable.data[c])->str;
}
