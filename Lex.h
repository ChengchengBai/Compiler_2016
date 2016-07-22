#pragma once
/******************************/
/*							  */
/*	��̬�ַ������ݽṹ���    */
/*	��Ӧ�ļ�DynString.cpp	  */
/******************************/

typedef struct DynString		//��̬�ַ�������
{
	int count;			//�ַ�������
	int capacity;		//�����ַ�������������
	char* data;			//ָ���ַ���ָ��
}DynString;

void dynstring_init(DynString* ptr, int initsize);			//��̬�ַ�����ʼ��
void dynstring_free(DynString* ptr);						//�ͷŶ�̬�ַ���
void dynstring_reset(DynString* ptr);						//���³�ʼ��
void dynstring_realloc(DynString* ptr, int new_size);		//���·�������
void dynstring_chcat(DynString* ptr, char ch);				//׷�ӵ����ַ�����̬�ַ���

/******************************/
/*							  */
/*	��̬�������ݽṹ���      */
/*	��Ӧ�ļ�DynArray.cpp	  */
/******************************/

typedef struct DynArray
{
	int count;			//��̬����Ԫ�ظ���
	int capacity;		//��̬���黺��������
	void** data;		//ָ������ָ�������
}DynArray;

void dynarray_init(DynArray* ptr, int initsize);			//��ʼ��
void dynarray_realloc(DynArray* ptr, int new_size);			//���·��䶯̬��������
void dynarray_add(DynArray* ptr, void* data);				//׷�Ӷ�̬����Ԫ��
void dynarray_free(DynArray* ptr);							//�ڴ��ͷ�
int dynarray_search(DynArray* ptr, int no);				    //��̬����Ԫ�ز���

/******************************/
/*							  */
/*		  ���ʱ�              */
/*	��Ӧ�ļ�TokenTable.cpp	  */
/******************************/

typedef struct TkWord
{
	int tkcode;						//���ʱ���
	struct TkWord *next;			//��ϣ��ͻ��ͬ���
	char *str;						//�����ַ���
	struct Symbol *sym_struct;
	struct Symbol *sym_identifier;
}TkWord;

TkWord * tkWord_direct_insert(TkWord* tp);			//�ؼ��֣������ֱ�ӷ��뵥�ʱ�
TkWord * tkword_search(char* p, int keyno);			//�ڵ��ʱ��в��ҵ���
TkWord* tkword_insert(char* p);						//��ʶ�����뵥�ʱ�
void *mallocz(int size);							//�����ڴ�


/******************************
*							  *
*    �ʷ�����ȫ�ֱ���         *
*							  *
******************************/

#define MAXKEY 1024

//extern TkWord *tk_hashtable[MAXKEY];		//���ʹ�ϣ��
//extern DynArray tktable;					//���ʱ���ñ�ʶ��
//extern DynString tkstr;						//�����ַ���
//extern DynString sourcestr;					//����Դ�ַ���
//extern int token;							//���ʱ���
//extern char ch;								//��ǰԴ���ַ�
//extern int lineno;							//�к�
//extern int tkvalue;							//����ֵ
//extern char filename[1024];					//�ļ���


/*	elf��ϣ��������,��Ӧ�ļ�ElfHash.cpp	*/

int elf_hash(char* key);

/******************************
*							  *
*    �����������           *
*	 ��ӦError.cpp			  *
******************************/

enum ErrorLevel			//����ȼ�
{
	LEVEL_WARNING,
	LEVEL_ERROR,
};

enum WorkState			//�������
{
	STATE_COMPILE,
	STATE_LINK,
};

void warning(char* fmt, ...);			//���뾯��
void error(char* fmt, ...);				//���������
void link_error(char* fmt, ...);		//���Ӵ���
void expect(char* msg);					//ȱ��ĳ���﷨�ɷ�
void skip(int c);						//������ǰ����
char* get_tkstr(int c);

/******************************
*							  *
*    �ʷ�����������           *
*	 ��ӦLex.cpp			  *
******************************/
enum LexState		//�ʷ�״̬
{
	LEX_NORMAL,
	LEX_SEP
};

void init_lex();					//�ʷ�������ʼ��
void getch();						//��Դ�ļ��ж�ȡһ���ַ�
void skip_space();					//�����հ��ַ�
void lexical_comment();				//����ע��
void preprocess();					//Ԥ�������Կհ׼�ע��
int is_letter(char c);				//�Ƿ�����ĸ�»���
int is_digit(char c);				//�Ƿ�������
void lexical_identifier();			//��ʶ���Ĵ���
void lexical_num();					//�������ͳ���
void lexical_string(char s);		//�����ַ���
void color_token(int lex_state);	//������ɫ
void get_token();					//ȡ����