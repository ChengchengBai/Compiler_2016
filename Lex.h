#pragma once
/******************************/
/*							  */
/*	动态字符串数据结构设计    */
/*	对应文件DynString.cpp	  */
/******************************/

typedef struct DynString		//动态字符串定义
{
	int count;			//字符串长度
	int capacity;		//包含字符串缓冲区长度
	char* data;			//指向字符串指针
}DynString;

void dynstring_init(DynString* ptr, int initsize);			//动态字符串初始化
void dynstring_free(DynString* ptr);						//释放动态字符串
void dynstring_reset(DynString* ptr);						//重新初始化
void dynstring_realloc(DynString* ptr, int new_size);		//重新分配容量
void dynstring_chcat(DynString* ptr, char ch);				//追加单个字符到动态字符串

/******************************/
/*							  */
/*	动态数组数据结构设计      */
/*	对应文件DynArray.cpp	  */
/******************************/

typedef struct DynArray
{
	int count;			//动态数组元素个数
	int capacity;		//动态数组缓冲区长度
	void** data;		//指向数据指针的数组
}DynArray;

void dynarray_init(DynArray* ptr, int initsize);			//初始化
void dynarray_realloc(DynArray* ptr, int new_size);			//重新分配动态数组容量
void dynarray_add(DynArray* ptr, void* data);				//追加动态数组元素
void dynarray_free(DynArray* ptr);							//内存释放
int dynarray_search(DynArray* ptr, int no);				    //动态数组元素查找

/******************************/
/*							  */
/*		  单词表              */
/*	对应文件TokenTable.cpp	  */
/******************************/

typedef struct TkWord
{
	int tkcode;						//单词编码
	struct TkWord *next;			//哈希冲突的同义词
	char *str;						//单词字符串
	struct Symbol *sym_struct;
	struct Symbol *sym_identifier;
}TkWord;

TkWord * tkWord_direct_insert(TkWord* tp);			//关键字，运算符直接放入单词表
TkWord * tkword_search(char* p, int keyno);			//在单词表中查找单词
TkWord* tkword_insert(char* p);						//标识符插入单词表
void *mallocz(int size);							//分配内存


/******************************
*							  *
*    词法分析全局变量         *
*							  *
******************************/

#define MAXKEY 1024

//extern TkWord *tk_hashtable[MAXKEY];		//单词哈希表
//extern DynArray tktable;					//单词表放置标识符
//extern DynString tkstr;						//单词字符串
//extern DynString sourcestr;					//单词源字符串
//extern int token;							//单词编码
//extern char ch;								//当前源码字符
//extern int lineno;							//行号
//extern int tkvalue;							//单词值
//extern char filename[1024];					//文件名


/*	elf哈希函数计算,对应文件ElfHash.cpp	*/

int elf_hash(char* key);

/******************************
*							  *
*    警告与错误处理           *
*	 对应Error.cpp			  *
******************************/

enum ErrorLevel			//错误等级
{
	LEVEL_WARNING,
	LEVEL_ERROR,
};

enum WorkState			//错误过程
{
	STATE_COMPILE,
	STATE_LINK,
};

void warning(char* fmt, ...);			//编译警告
void error(char* fmt, ...);				//编译错误处理
void link_error(char* fmt, ...);		//链接错误
void expect(char* msg);					//缺少某个语法成分
void skip(int c);						//跳过当前符号
char* get_tkstr(int c);

/******************************
*							  *
*    词法分析主程序           *
*	 对应Lex.cpp			  *
******************************/
enum LexState		//词法状态
{
	LEX_NORMAL,
	LEX_SEP
};

void init_lex();					//词法分析初始化
void getch();						//从源文件中读取一个字符
void skip_space();					//跳过空白字符
void lexical_comment();				//处理注释
void preprocess();					//预处理，忽略空白及注释
int is_letter(char c);				//是否是字母下划线
int is_digit(char c);				//是否是数字
void lexical_identifier();			//标识符的处理
void lexical_num();					//解析整型常量
void lexical_string(char s);		//解析字符串
void color_token(int lex_state);	//单词着色
void get_token();					//取单词