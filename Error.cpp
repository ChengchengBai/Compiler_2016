/*
		Error.cpp
		错误与异常处理
*/

#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include"Lex.h"
#include"BCC.h"

void warning(char* fmt, ...)		//编译警告
{
	va_list ap;
	va_start(ap, fmt);

	char buf[1024];
	vsprintf(buf, fmt, ap);

	printf("\n%s(第%d行)：编译警告：%s!\n", filename, lineno, buf);

	va_end(ap);
}

void error(char* fmt, ...)			//编译错误处理
{
	va_list ap;
	va_start(ap, fmt);

	char buf[1024];
	vsprintf(buf, fmt, ap);

	printf("\n%s(第%d行)：编译错误：%s!\n", filename, lineno, buf);
	system("pause");
	exit(-1);

	va_end(ap);
}

void link_error(char* fmt, ...)		//链接错误
{
	va_list ap;
	va_start(ap, fmt);

	char buf[1024];
	vsprintf(buf, fmt, ap);

	printf("\t链接错误：%s！\n", buf);
	system("pause");
	exit(-1);

}

void expect(char* msg)				//缺少某个语法成分
{
	error("\t缺少%s", msg);
}

void skip(int c)					//跳过当前符号
{
	if (token != c)
		error("\t缺少'%s'", get_tkstr(c));
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
