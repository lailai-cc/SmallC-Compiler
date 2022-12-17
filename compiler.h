#ifndef COMPILER_H
#define COMPILER_H

#define bool int
#define true 1
#define false 0

#define norw 20       /* 保留字个数 */
#define txmax 100     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 10         /* 标识符的最大长度 */
#define maxerr 30     /* 允许的最多错误数 */
#define amax 2048     /* 地址上界*/
#define levmax 3      /* 最大允许过程嵌套声明层数*/
#define cxmax 200     /* 最多的虚拟机代码数 */
#define stacksize 500 /* 运行时数据栈元素最多为500个 */

/* 符号 */
enum symbol {
    nul,         ID,    //标识符
    NUM,   //无符号数字
    plus,      minus,
    times,       slash,     oddsym,     eql,       neq,
    lss,         leq,       gtr,        geq,       lparen,
    rparen,      comma,     semicolon,  period,    becomes,
    beginsym,    endsym,    ifsym,      thensym,   whilesym,
    writesym,    readsym,   dosym,      callsym,   constsym,
    intsym,      procsym,   lbrace,     rbrace,    mod,
    elsesym,	 forsym,    boolsym,    andsym,    orsym,
    notsym,      xorsym,    exc,		breaksym,  continuesym,
    exitsym,	falsesym,	truesym
};
#define symnum 48

/* 符号表中的类型 */
enum object {
    constant,
    intnum,
    boolean,
    //procedure,
};

/* 虚拟机代码指令 */
enum fct {
    lit,     opr,     lod,
    sto,     cal,     ini,
    jmp,     jpc,
};
#define fctnum 8

/* 虚拟机代码结构 */
struct instruction
{
    enum fct f; /* 虚拟机代码指令 */
    int l;      /* 引用层与声明层的层次差 */
    int a;      /* 根据f的不同而不同 */
};

/* 符号表结构 */
struct tablestruct
{
    char name[al];	    /* 名字 */
    enum object kind;	/* 类型：const，var或procedure */
    int val;            /* 数值，仅const使用 */
    int level;          /* 所处层，仅const不使用 */
    int adr;            /* 地址，仅const不使用 */
    int size;           /* 需要分配的数据区空间, 仅procedure使用 */
};

void error(int n);
void getsym();
void getch();
void init();
void gen(enum fct x, int y, int z);
void test(bool* s1, bool* s2, int n);
int inset(int e, bool* s);
int addset(bool* sr, bool* s1, bool* s2, int n);
int subset(bool* sr, bool* s1, bool* s2, int n);
int mulset(bool* sr, bool* s1, bool* s2, int n);
void block(int lev, int tx, bool* fsys);
bool interpret(int step,int sum);
void factor(bool* fsys, int* ptx, int lev);
void term(bool* fsys, int* ptx, int lev);
//void condition(bool* fsys, int* ptx, int lev);
void expression(bool* fsys, int* ptx, int lev);
void statement(bool* fsys, int* ptx, int lev);
void listcode(int cx0);
void listall();
void booldeclaration(int* ptx, int lev, int* pdx);
void intdeclaration(int* ptx, int lev, int* pdx);
void constdeclaration(int* ptx, int lev, int* pdx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, int* s, int b);

void additive_expr(bool* fsys, int* ptx, int lev);
void bool_expr(bool* fsys, int* ptx, int lev);
void if_stat(bool* fsys, int* ptx, int lev);
void while_stat(bool* fsys, int* ptx, int lev);
void write_stat(bool* fsys, int* ptx, int lev);
void read_stat(bool* fsys, int* ptx, int lev);
void compound_stat(bool* fsys, int* ptx, int lev);
void expression_stat(bool* fsys, int* ptx, int lev);
int compiler();
#endif // COMPILER_H
