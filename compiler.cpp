/*
 * PL/0 complier program implemented in C
 *
 * The program has been tested on Visual Studio 2010
 *
 * 使用方法：
 * 运行后输入PL/0源程序文件名
 * 回答是否输出虚拟机代码
 * 回答是否输出符号表
 * fcode.txt输出虚拟机代码
 * foutput.txt输出源文件、出错示意（如有错）和各行对应的生成代码首地址（如无错）
 * fresult.txt输出运行结果
 * ftable.txt输出符号表
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "compiler.h"


bool listswitch ;   /* 显示虚拟机代码与否 */
bool tableswitch ;  /* 显示符号表与否 */
char ch;            /* 存放当前读取的字符，getch 使用 */
enum symbol sym;    /* 当前的符号 */
char id[al+1];      /* 当前ident，多出的一个字节用于存放0 */
int num;            /* 当前number */
int cc, ll;         /* getch使用的计数器，cc表示当前字符(ch)的位置 */
int cx;             /* 虚拟机代码指针, 取值范围[0, cxmax-1]*/
char line[81];      /* 读取行缓冲区 */
char a[al+1];       /* 临时符号，多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[norw][al];        /* 保留字 */
enum symbol wsym[norw];     /* 保留字对应的符号值 */
enum symbol ssym[256];      /* 单字符的符号值 */
char mnemonic[fctnum][5];   /* 虚拟机代码指令名称 */
bool declbegsys[symnum];    /* 表示声明开始的符号集合 */
bool statbegsys[symnum];    /* 表示语句开始的符号集合 */
bool facbegsys[symnum];     /* 表示因子开始的符号集合 */

bool jmploop[10];             /* 用于判断statement_list语句中是否可能有continue和break语句 */

int circlelev;               /*当前处于第几层loop*/
int continue_cx[10][100];   /* continue语句位置 */
int continue_n[10];         /* continue语句个数 */
int break_cx[10][100];      /* break语句位置 */
int break_n[10];            /* break语句个数 */
int exit_cx[100];           /* exit语句位置(最多100处) */
int exit_n = 0;             /* exit语句个数 */

bool fend = false;            /*文件是否读完*/


struct tablestruct table[txmax]; /* 符号表 */

FILE* fin;      /* 输入源文件 */
FILE* ftable;	/* 输出符号表 */
FILE* fcode;    /* 输出虚拟机代码 */
FILE* foutput;  /* 输出文件及出错示意（如有错）、各行对应的生成代码首地址（如无错） */
FILE* fresult;  /* 输出执行结果 */
FILE* fstack; //输出数据栈
FILE* fsinput;  //标准输入

char fname[al];
int err;        /* 错误计数器 */

/* 主程序开始 */
int compiler()
{
    bool nxtlev[symnum];

    //printf("Input pl/0 file?   ");
    //scanf("%s", fname);		/* 输入文件名 */

    if ((fin = fopen("input.txt", "r")) == NULL)
    {
        printf("Can't open the input file!\n");
        exit(1);
    }

    ch = fgetc(fin);
    if (ch == EOF)
    {
        printf("The input file is empty!\n");
        fclose(fin);
        exit(1);
    }
    rewind(fin);

    if ((foutput = fopen("../foutput.txt", "w")) == NULL)
    {
        printf("Can't open the output file!\n");
        exit(1);
    }

    if ((ftable = fopen("../ftable.txt", "w")) == NULL)
    {
        printf("Can't open ftable.txt file!\n");
        exit(1);
    }

    if ((fsinput= fopen("sinput.txt", "r")) == NULL)
    {
        printf("Can't open the sinput file!\n");
        exit(1);
    }

    // printf("List object codes?(Y/N)");	/* 是否输出虚拟机代码 */
    // scanf("%s", fname);
    // listswitch = (fname[0]=='y' || fname[0]=='Y');
    listswitch=1;

    // printf("List symbol table?(Y/N)");	/* 是否输出符号表 */
    // scanf("%s", fname);
    // tableswitch = (fname[0]=='y' || fname[0]=='Y');
    tableswitch = 1;


    init();		/* 初始化 */

    err = 0;
    cc = ll = cx = 0;
    ch = ' ';

    getsym();
    if(sym != lbrace){
        error(0);  /* 缺少{ */
        while (!inset(sym, nxtlev))
        {
            getsym();
        }
    }
    else{
        getsym();
    }
    addset(nxtlev, declbegsys, statbegsys, symnum);

    block(0, 0, nxtlev);	/* 处理分程序 */

    if (sym == rbrace)
    {
        getsym();
    }
    else{
        error(1);  /* 缺少} */
    }

    if (err == 0)
    {
        printf("\n===Parsing success!===\n");
        fprintf(foutput,"\n===Parsing success!===\n");

        if ((fcode = fopen("../fcode.txt", "w")) == NULL)
        {
            printf("Can't open fcode.txt file!\n");
            exit(1);
        }

        if ((fresult = fopen("../fresult.txt", "w")) == NULL)
        {
            printf("Can't open fresult.txt file!\n");
            exit(1);
        }

        listall();	 /* 输出所有代码 */
        fclose(fcode);

        interpret(0,0);	/* 调用解释执行程序 */
        fclose(fresult);
      }
    else
    {
        printf("\n%d errors in pl/0 program!\n",err);
        fprintf(foutput,"\n%d errors in pl/0 program!\n",err);
    }

    fclose(ftable);
    fclose(foutput);
    fclose(fin);

    return 0;
}

/*
 * 初始化
 */
void init()
{
    int i;

    /* 设置单字符符号 */
    for (i=0; i<=255; i++)
    {
        ssym[i] = nul;
    }
    ssym['+'] = plus;
    ssym['-'] = minus;
    ssym['*'] = times;
    ssym['/'] = slash;
    ssym['('] = lparen;
    ssym[')'] = rparen;
    ssym['='] = becomes;
    ssym[','] = comma;
    ssym['.'] = period;
    ssym[';'] = semicolon;
    //mine
    ssym['!'] = exc; //exclamation
    ssym['%'] = mod;
    ssym['{'] = lbrace;
    ssym['}'] = rbrace;

    /* 设置保留字名字,按照字母顺序，便于二分查找 */
    strcpy(&(word[0][0]), "and");
    strcpy(&(word[1][0]), "bool");
    strcpy(&(word[2][0]), "break");
    strcpy(&(word[3][0]), "const");
    strcpy(&(word[4][0]), "continue");
    strcpy(&(word[5][0]), "do");
    strcpy(&(word[6][0]), "else");
    strcpy(&(word[7][0]), "exit");
    strcpy(&(word[8][0]), "false");
    strcpy(&(word[9][0]), "for");
    strcpy(&(word[10][0]), "if");
    strcpy(&(word[11][0]), "int");
    strcpy(&(word[12][0]), "not");
    strcpy(&(word[13][0]), "odd");
    strcpy(&(word[14][0]), "or");
    strcpy(&(word[15][0]), "read");
    strcpy(&(word[16][0]), "true");
    strcpy(&(word[17][0]), "while");
    strcpy(&(word[18][0]), "write");
    strcpy(&(word[19][0]), "xor");


    /* 设置保留字符号 */
    wsym[0] = andsym;
    wsym[1] = boolsym;
    wsym[2] = breaksym;
    wsym[3] = constsym;
    wsym[4] = continuesym;
    wsym[5] = dosym;
    wsym[6] = elsesym;
    wsym[7] = exitsym;
    wsym[8] = falsesym;
    wsym[9] = forsym;
    wsym[10] = ifsym;
    wsym[11] = intsym;
    wsym[12] = notsym;
    wsym[13] = oddsym;
    wsym[14] = orsym;
    wsym[15] = readsym;
    wsym[16] = truesym;
    wsym[17] = whilesym;
    wsym[18] = writesym;
    wsym[19] = xorsym;


    /* 设置指令名称 */
    strcpy(&(mnemonic[lit][0]), "lit");
    strcpy(&(mnemonic[opr][0]), "opr");
    strcpy(&(mnemonic[lod][0]), "lod");
    strcpy(&(mnemonic[sto][0]), "sto");
    strcpy(&(mnemonic[cal][0]), "cal");
    strcpy(&(mnemonic[ini][0]), "int");
    strcpy(&(mnemonic[jmp][0]), "jmp");
    strcpy(&(mnemonic[jpc][0]), "jpc");

    /* 设置符号集 */
    for (i=0; i<symnum; i++)
    {
        declbegsys[i] = false;
        statbegsys[i] = false;
        facbegsys[i] = false;
    }

    /* 设置声明开始符号集 */
    declbegsys[constsym] = true;
    declbegsys[intsym] = true;
    declbegsys[boolsym] = true;


    /* 设置语句开始符号集 */
    statbegsys[lbrace] = true;
    statbegsys[lparen] = true;
    statbegsys[NUM] = true;
    statbegsys[ID] = true;
    statbegsys[writesym] = true;
    statbegsys[readsym] = true;
    statbegsys[ifsym] = true;
    statbegsys[whilesym] = true;

    /* 设置因子开始符号集 */
    facbegsys[ID] = true;
    facbegsys[NUM] = true;
    facbegsys[lparen] = true;
    facbegsys[falsesym] = true;
    facbegsys[truesym] = true;
    facbegsys[notsym] = true;

    /* 初始化break, continue记录个数为零,
       初始化标记为不在循环体中 */
    for(i = 0; i < 10; i++)
    {
        break_n[i] = 0;
        jmploop[i] = false;
        continue_n[i] = 0;
    }
}

/*
 * 用数组实现集合的集合运算
 */
int inset(int e, bool* s)
{
    return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]||s2[i];
    }
    return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]&&(!s2[i]);
    }
    return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]&&s2[i];
    }
    return 0;
}

/*
 *	出错处理，打印出错位置和错误编码
 */
void error(int n)
{
    char space[81];
    memset(space,32,81);

    space[cc-1]=0; /* 出错时当前符号已经读完，所以cc-1 */

    printf("**%s^%d\n", space, n);
    fprintf(foutput,"**%s^%d\n", space, n);

    err = err + 1;
    if (err > maxerr)
    {
        exit(1);
    }
}

/*
 * 过滤空格，读取一个字符
 * 每次读一行，存入line缓冲区，line被getsym取空后再读一行
 * 被函数getsym调用
 */
void getch()
{
    bool havechar =false; //该行是否有有效字符
    if (cc == ll) /* 判断缓冲区中是否有字符，若无字符，则读入下一行字符到缓冲区中 */
    {
        if (feof(fin))
        {
            fend = true;
            // printf("Program is incomplete!\n");
            // exit(1);
        }
        ll = 0;
        cc = 0;
        ch = ' ';
        char pre = ' ';
        while (ch != 10)  /*换行键*/
        {
            pre = ch;
            if (EOF == fscanf(fin,"%c", &ch))  //如果文件结束
            {
                if(ll!=0){
                    line[ll] = 0;
                }
                fend=true;
                break;
            }
            if(pre=='/'){
                if(ch!='*'){   //说明不是注释，要多输出一个/
                    if(!havechar){
                        printf("%d ", cx);
                        fprintf(foutput,"%d ", cx);
                        havechar=true;
                    }

                    printf("%c", pre);
                    fprintf(foutput, "%c", pre);
                    line[ll] = pre;
                    ll++;

                    if(ch!='/'){
                        if(!havechar){
                            printf("%d ", cx);
                            fprintf(foutput,"%d ", cx);
                            havechar=true;
                        }

                        printf("%c", ch);
                        fprintf(foutput, "%c", ch);
                        line[ll] = ch;
                        ll++;
                    }
                }
                else {
                    line[ll] = 0;     //该句代码结束
                    bool findend=false;
                    pre=ch;
                    while(EOF!=fscanf(fin,"%c", &ch)){
                        //printf("%c%c\n", pre,ch);
                        if(pre=='*'&& ch =='/'){
                            findend=true;
                            break;
                        }
                        pre=ch;
                    }

                    if(ll == 0)                         // 该行头两个字符就是"/*"
                    {
                        ch = ' ';
                        continue;
                    }

                    if(!findend){
                        error(29);      // 注释块缺少*/
                    }
                    break;   //前面已经设置line[ll] = 0;这里必须要调出循环，读取下一句代码
                }
            }
            else if(ch!='/'){
                if(!havechar){
                    printf("%d ", cx);
                    fprintf(foutput,"%d ", cx);
                    havechar=true;
                }

                printf("%c", ch);
                fprintf(foutput, "%c", ch);
                line[ll] = ch;
                ll++;
            }
        }
    }
    if(fend==true && ll==0){
        ch=0;
    }
    else{            //有缓存
        ch = line[cc];
        cc++;
    }

}

/*
 * 词法分析，获取一个符号
 */
void getsym()
{
    //printf("getsym%c",ch);
    int i,j,k;

    while (ch == ' ' || ch == 10 || ch == 9)	/* 过滤空格、换行和制表符 */
    {
        getch();
    }

    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* 当前的单词是标识符或是保留字 */
    {
        k = 0;
        do {
            if(k < al)
            {
                a[k] = ch;
                k++;
            }
            getch();
        } while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));
        a[k] = 0;
        strcpy(id, a);
        i = 0;
        j = norw - 1;
        //printf("\n%s\n",id);
        do {    /* 搜索当前单词是否为保留字，使用二分法查找 */
            k = (i + j) / 2;
            if (strcmp(id,word[k]) <= 0)
            {
                j = k - 1;
            }
            if (strcmp(id,word[k]) >= 0)
            {
                i = k + 1;
            }
        } while (i <= j);
        if (i-1 > j) /* 当前的单词是保留字 */
        {
            sym = wsym[k];
            //printf("kkk");
        }
        else /* 当前的单词是标识符 */
        {
            sym = ID;
            //printf("id");
        }
    }
    else
    {
        if (ch >= '0' && ch <= '9') /* 当前的单词是数字 */
        {
            k = 0;
            num = 0;
            sym = NUM;
            //printf("num\n");
            do {
                num = 10 * num + ch - '0';
                k++;
                getch();;
            } while (ch >= '0' && ch <= '9'); /* 获取数字的值 */
            k--;
            if (k > nmax) /* 数字位数太多 */
            {
                error(30);
            }
        }
        else
        {
            if (ch == '=')		/* 检测赋值符号 */
            {
                getch();
                if (ch == '=')
                {
                    sym = eql;
                    getch();
                }
                else
                {
                    sym = becomes;
                    //printf("becomes");
                }
            }
            else
            {
                if (ch == '<')		/* 检测小于或小于等于符号 */
                {
                    getch();
                    if (ch == '=')
                    {
                        sym = leq;
                        getch();
                    }
                    else
                    {
                        sym = lss;
                    }
                }
                else
                {
                    if (ch == '>')		/* 检测大于或大于等于符号 */
                    {
                        getch();
                        if (ch == '=')
                        {
                            sym = geq;
                            getch();
                        }
                        else
                        {
                            sym = gtr;
                        }
                    }
                    else
                    {
                        if(ch == '!')
                        {
                            getch();
                            if(ch =='='){
                                sym = neq;
                                getch();
                            }
                            else{
                                sym = exc; //感叹号
                            }
                        }
                        else {
                            sym = ssym[ch];		/* 当符号不满足上述条件时，全部按照单字符符号处理 */
                            // if (sym != period)
                            // {
                                getch();
                            // }
                        }
                    }
                }
            }
        }
    }
}

/*
 * 生成虚拟机代码
 *
 * x: instruction.f;
 * y: instruction.l;
 * z: instruction.a;
 */
void gen(enum fct x, int y, int z )
{
    if (cx >= cxmax)
    {
        printf("Program is too long!\n");	/* 生成的虚拟机代码程序过长 */
        exit(1);
    }
    if ( z >= amax)
    {
        printf("Displacement address is too big!\n");	/* 地址偏移越界 */
        exit(1);
    }
    code[cx].f = x;
    code[cx].l = y;
    code[cx].a = z;
    cx++;
}


/*
 * 测试当前符号是否合法
 *
 * 在语法分析程序的入口和出口处调用测试函数test，
 * 检查当前单词进入和退出该语法单位的合法性
 *
 * s1:	需要的单词集合
 * s2:	如果不是需要的单词，在某一出错状态时，
 *      可恢复语法分析继续正常工作的补充单词符号集合
 * n:  	错误号
 */
void test(bool* s1, bool* s2, int n)
{
    if (!inset(sym, s1))
    {
        error(n);
        /* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
        while ((!inset(sym,s1)) && (!inset(sym,s2)))
        {
            //printf("test\n%d",sym);
            getsym();
        }
    }
}

void block(int lev, int tx, bool* fsys)
{   //包含declaration_list和statement_list
    int i;

    int dx;                 /* 记录数据分配的相对地址 */
    int tx0;                /* 保留初始tx */
    int cx0;                /* 保留初始cx */
    bool nxtlev[symnum];    /* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
                               传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空间
                               传递给下级函数*/

    dx = 3;                 /* 三个空间用于存放静态链SL、动态链DL和返回地址RA  */
    tx0 = tx;		        /* 记录本层标识符的初始位置 */
    table[tx].adr = cx;	    /* 记录当前层代码的开始位置 */
    gen(jmp, 0, 0);         /* 产生跳转指令，跳转位置未知暂时填0 */

    if (lev > levmax)		/* 嵌套层数过多 */
    {
        error(32);
    }

    while (inset(sym, declbegsys))	/* 直到没有声明符号 */
    {
       // printf("declaration_list");
        if (sym == constsym)	/* 遇到常量声明符号，开始处理常量声明 */
        {
            getsym();

            do {
                constdeclaration(&tx, lev, &dx);	/* dx的值会被constdeclaration改变，使用指针 */
                while (sym == comma)  /* 遇到逗号继续定义常量 */
                {
                    getsym();
                    constdeclaration(&tx, lev, &dx);
                }
                if (sym == semicolon) /* 遇到分号结束定义常量 */
                {
                    getsym();
                }
                else
                {
                    error(5);   /* 漏掉了分号 */
                }
            } while (sym == constsym);
        }

        if (sym == intsym)		/* 遇到变量声明符号，开始处理变量声明 */
        {
            getsym();

            do {
                intdeclaration(&tx, lev, &dx);
                while (sym == comma)
                {
                    getsym();
                    intdeclaration(&tx, lev, &dx);
                }
                if (sym == semicolon)
                {
                    getsym();
                }
                else
                {
                    error(5); /* 漏掉了分号 */
                }
            } while (sym == intsym);
        }
        if (sym == boolsym)		/* 遇到变量声明符号，开始处理变量声明 */
        {
            getsym();

            do {
                booldeclaration(&tx, lev, &dx);
                while (sym == comma)
                {
                    getsym();
                    booldeclaration(&tx, lev, &dx);
                }
                if (sym == semicolon)
                {
                    getsym();
                }
                else
                {
                    error(5); /* 漏掉了分号 */
                }
            } while (sym == boolsym);
        }

        memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
        nxtlev[ID] = true;
        nxtlev[constsym] = true;
        test(nxtlev, declbegsys, 7);
    }

    code[table[tx0].adr].a = cx;	/* 把前面生成的跳转语句的跳转位置改成当前位置 */
    table[tx0].adr = cx;	        /* 记录当前过程代码地址 */
    table[tx0].size = dx;	        /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
    cx0 = cx;
    gen(ini, 0, dx);	            /* 生成指令，此指令执行时在数据栈中为被调用的过程开辟dx个单元的数据区 */

    if (tableswitch)		/* 输出符号表 */
    {
        for (i = 1; i <= tx; i++)
        {
            switch (table[i].kind)
            {
                case constant:
                    printf("    %d const %s ", i, table[i].name);
                    printf("val=%d\n", table[i].val);
                    fprintf(ftable, "    %d const %s ", i, table[i].name);
                    fprintf(ftable, "val=%d\n", table[i].val);
                    break;
                case intnum:
                    printf("    %d int   %s ", i, table[i].name);
                    printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
                    fprintf(ftable, "    %d var   %s ", i, table[i].name);
                    fprintf(ftable, "lev=%d addr=%d\n", table[i].level, table[i].adr);
                    break;
                case boolean:
                    printf("    %d bool  %s ", i, table[i].name);
                    printf("lev=%d addr=%d \n", table[i].level, table[i].adr);
                    fprintf(ftable,"    %d bool  %s ", i, table[i].name);
                    fprintf(ftable,"lev=%d addr=%d\n", table[i].level, table[i].adr);
                    break;
            }
        }
        printf("\n");
        fprintf(ftable,"\n");
    }

    /* 语句后继符号为分号或end */
    memcpy(nxtlev, fsys, sizeof(bool) * symnum);	/* 每个后继符号集合都包含上层后继符号集合，以便补救 */
    nxtlev[semicolon] = true;
    nxtlev[rbrace] = true;
    nxtlev[elsesym] = true;

    //statement_list
    while (inset(sym, statbegsys)|| ( (sym == continuesym || sym == breaksym ) && jmploop[circlelev] == true ))	/* 直到没有语句开始符号 */
    {
        //printf("statemnet");
        statement(nxtlev, &tx, lev);
    }
    //printf("hhhhhh block end\n");
    gen(opr, 0, 0);	                    /* 每个过程出口都要使用的释放数据段指令 */
    memset(nxtlev, 0, sizeof(bool) * symnum);	/* 分程序没有补救集合 */
    nxtlev[rbrace] = true;
    nxtlev[nul]=true;
    test(nxtlev, nxtlev, 8);            	/* 检测后继符号正确性 */
    listcode(cx0);                      /* 输出本分程序生成的代码 */
}

/*
 * 在符号表中加入一项
 *
 * k:      标识符的种类为const，var或procedure
 * ptx:    符号表尾指针的指针，为了可以改变符号表尾指针的值
 * lev:    标识符所在的层次
 * pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
 *
 */
void enter(enum object k, int* ptx,	int lev, int* pdx)
{
    (*ptx)++;
    strcpy(table[(*ptx)].name, id); /* 符号表的name域记录标识符的名字 */
    table[(*ptx)].kind = k;
    switch (k)
    {
        case constant:	/* 常量 */
            if (num > amax)
            {
                error(31);	/* 常数越界 */
                num = 0;
            }
            table[(*ptx)].val = num; /* 登记常数的值 */
            break;
        case intnum:	/* 整形变量 */
            table[(*ptx)].level = lev;
            table[(*ptx)].adr = (*pdx);
            (*pdx)++;
            break;
        case boolean:
            table[(*ptx)].level = lev;
            table[(*ptx)].adr = (*pdx);
            (*pdx)++;
            break;
        // case procedure:	/* 过程 */
        // 	table[(*ptx)].level = lev;
        // 	break;
    }
}

/*
 * 查找标识符在符号表中的位置，从tx开始倒序查找标识符
 * 找到则返回在符号表中的位置，否则返回0
 *
 * id:    要查找的名字
 * tx:    当前符号表尾指针
 */
int position(char* id, int tx)
{
    int i;
    strcpy(table[0].name, id);
    i = tx;
    while (strcmp(table[i].name, id) != 0)
    {
        i--;
    }
    return i;
}

/*
 * 常量声明处理
 */
void constdeclaration(int* ptx, int lev, int* pdx)
{
    if (sym == ID)
    {
        getsym();
        if (sym == eql || sym == becomes)
        {
            if (sym == eql)
            {
                error(28);	/* 把=写成了== */
            }
            getsym();
            if (sym == NUM)
            {
                enter(constant, ptx, lev, pdx);
                getsym();
            }
            else
            {
                error(2);	/* 常量声明中的=后应是数字 */
            }
        }
        else
        {
            error(3);	/* 常量声明中的标识符后应是= */
        }
    }
    else
    {
        error(4);	/* const后应是标识符 */
    }
}

/*
 * 整型变量声明处理
 */
void intdeclaration(int* ptx,int lev,int* pdx)
{
    if (sym == ID)
    {
        enter(intnum, ptx, lev, pdx);	// 填写符号表
        getsym();
    }
    else
    {
        error(4);	/* int后面应是标识符 */
    }
}

/*
 * 布尔变量声明处理
 */
void booldeclaration(int* ptx,int lev,int* pdx)
{
    if (sym == ID)
    {
        enter(boolean, ptx, lev, pdx);	// 填写符号表
        getsym();
    }
    else
    {
        error(4);	/* bool后面应是标识符 */
    }
}

/*
 * 输出目标代码清单
 */
void listcode(int cx0)
{
    int i;
    if (listswitch)
    {
        printf("\n");
        for (i = cx0; i < cx; i++)
        {
            printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
        }
    }
}

/*
 * 输出所有目标代码
 */
void listall()
{
    int i;
    if (listswitch)
    {
        for (i = 0; i < cx; i++)
        {
            printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
            fprintf(fcode,"%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
        }
    }
}

/*
 * 语句处理
 */
void statement(bool* fsys, int* ptx, int lev)
{
    //printf("statement hhhh %d\n",sym);
    bool nxtlev[symnum];

    if (sym == ifsym)	/* 准备按照赋值语句处理 */
    {
        if_stat(fsys, ptx, lev);
    }
    else
    {
        if(sym == whilesym){
            while_stat(fsys, ptx, lev);
        }
        else if(sym == writesym){
          //  printf("writement");
            write_stat(fsys, ptx, lev);
        }
        else if (sym == readsym)	/* 准备按照read语句处理 */
        {
            read_stat(fsys, ptx, lev);
        }
        else if(sym == lbrace){
            compound_stat(fsys, ptx, lev);
        }
        else if (sym == continuesym && jmploop[circlelev] == true)
        {
            continue_cx[circlelev][continue_n[circlelev]] = cx;
            continue_n[circlelev] = continue_n[circlelev]+1;
            gen(jmp, 0, 0);
            getsym();
            if(sym == semicolon){
                getsym();
            }
            else{
                error(5);   /* 漏掉了分号 */
            }

        }
        else if (sym == breaksym && jmploop[circlelev] == true)
        {
            break_cx[circlelev][break_n[circlelev]] = cx;
            break_n[circlelev] = break_n[circlelev]+1;
            gen(jmp, 0, 0);
            getsym();
            if(sym == semicolon){
                getsym();
            }
            else{
                error(5);   /* 漏掉了分号 */
            }
        }
        else if (sym == ID || sym == NUM || sym == lparen || sym == semicolon){
            expression_stat(fsys, ptx, lev);
        }
    }
    memset(nxtlev, 0, sizeof(bool) * symnum);	/* 语句结束无补救集合 */
    nxtlev[nul]=true;
    test(fsys, nxtlev, 19);	/* 检测语句结束的正确性 */
}


/*
 * if_stat处理
 */
void if_stat(bool* fsys, int* ptx, int lev){
  //  printf("if_stat\n");
    bool nxtlev[symnum];
    int cx1,cx2;

    getsym();
    if(sym==lparen){
        getsym();
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[rparen] = true;	/* 后继符号为then或do */
        expression(nxtlev, ptx, lev); /* 调用条件处理 */
        if (sym == rparen)
        {
            getsym();
        }
        else
        {
            error(12);	/* 缺少) */
        }
        cx1 = cx;	/* 保存当前指令地址 */
        gen(jpc, 0, 0);	/* 生成条件跳转指令，跳转地址未知，暂时写0 */
     //   printf("if state\n");
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[elsesym] = true;
        statement(nxtlev, ptx, lev);	/* 满足条件执行 */

        if(sym == elsesym){
            // cx2 = cx;	        /* 保存当前指令地址 */
            // gen(jmp, 0, 0);	    /* 生成条件跳转指令，跳转地址未知，暂时写0 */
            code[cx1].a = cx;	/* 经statement处理后，cx为else后语句的位置，它正是前面未定的跳转地址，此时进行回填 */
            getsym();
            statement(fsys, ptx, lev);	/* 处理语句 */
            // code[cx2].a = cx;	/* 经statement处理后，cx为else后语句执行完的位置，它正是前面未定的跳转地址，此时进行回填 */
        }
        else{
            code[cx1].a = cx;	/* 经statement处理后，cx为整个if语句执行完的位置，它正是前面未定的跳转地址，此时进行回填 */
        }
    }
}

/*
 * while_stat处理
 */
void while_stat(bool* fsys, int* ptx, int lev){
    bool nxtlev[symnum];
    int cx1,cx2;

    cx1 = cx;	/* 保存判断条件操作的位置 */
    circlelev += 1;
    getsym();
    if(sym==lparen){
        getsym();
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[rparen] = true;	/* 后继符号为do */
        expression(nxtlev, ptx, lev);	/* 调用条件处理 */
        cx2 = cx;	/* 保存循环体的结束的下一个位置 */
        gen(jpc, 0, 0);	/* 生成条件跳转，但跳出循环的地址未知，标记为0等待回填 */
        if (sym == rparen)
        {
            getsym();
        }
        else
        {
            error(12);	/* 缺少) */
        }


        fsys[continuesym] = true;
        fsys[breaksym] = true;
        jmploop[circlelev] = true;

        statement(fsys, ptx, lev);	/* 循环体 */

        jmploop[circlelev] = false;

        gen(jmp, 0, cx1);	/* 生成条件跳转指令，跳转到前面判断条件操作的位置 */
        code[cx2].a = cx;	/* 回填跳出循环的地址 */

        while(break_n[circlelev]>0){
            code[break_cx[circlelev][break_n[circlelev]-1]].a = cx;
            break_n[circlelev] = break_n[circlelev]-1;
        }

        while (continue_n[circlelev] > 0)
        {
            code[continue_cx[circlelev][continue_n[circlelev]-1]].a = cx1;
            continue_n[circlelev] = continue_n[circlelev]-1;
        }
        circlelev -= 1;
    }
    else{
        error(11); /* 缺少( */
    }
}

/*
 * write_stat处理
 */
void write_stat(bool* fsys, int* ptx, int lev){
  //  printf("write_stat");
    bool nxtlev[symnum];
    do {
        getsym();
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[semicolon] = true;
        nxtlev[comma] = true;
        expression(nxtlev, ptx, lev);	/* 调用表达式处理 */
        gen(opr, 0, 14);	/* 生成输出指令，输出栈顶的值 */
        gen(opr, 0, 15);	/* 生成换行指令 */
    } while (sym == comma);  /* 一条write可输出多个变量的值 */

    if(sym == semicolon){
        getsym();
    }
    else{
        error(5);   /* 漏掉了分号 */
    }
}

/*
 * read_stat处理
 */
void read_stat(bool* fsys, int* ptx, int lev){
    int i;
    do{
        getsym();
        if (sym == ID)
        {
            i = position(id, *ptx);	/* 查找要读的变量 */
        }
        else{
            i = 0;
        }

        if (i == 0){
            error(35);	/* read语句括号中的标识符应该是声明过的变量 */
        }
        else{
            gen(opr, 0, 16);	/* 生成输入指令，读取值到栈顶 */
            gen(sto, lev-table[i].level, table[i].adr);	/* 将栈顶内容送入变量单元中 */
        }

        getsym();
    } while (sym == comma);	/* 一条read语句可读多个变量 */

    if(sym == semicolon){
        getsym();
    }
    else{
        error(5);   /* 漏掉了分号 */
    }
}

/*
 * compound_stat处理
 */
void compound_stat(bool* fsys, int* ptx, int lev){
   // printf("compound_stat\n");
    bool nxtlev[symnum];

    if (sym == lbrace)	/* { } */
    {
        getsym();
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[rbrace] = true;
        //statement_list
        while (inset(sym, statbegsys) || ( (sym == continuesym || sym == breaksym ) && jmploop[circlelev] == true ))	/* 直到没有语句开始符号 */
        {
            statement(nxtlev, ptx, lev);
        }

        if (sym == rbrace)
        {
            getsym();
        }
        else
        {
            error(1);	/* 缺少右大括号 */
        }
    }
}

/*
 * expression_stat处理
 */
void expression_stat(bool* fsys, int* ptx, int lev){
    bool nxtlev[symnum];

    if(sym == semicolon)	/*遇到分号，结束表达式*/
    {
        getsym();
    }
    else{
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[semicolon] = true;
        expression(nxtlev, ptx, lev);
        if(sym == semicolon)	/*遇到分号，结束表达式*/{
         //   printf("expression end\n");
            getsym();
        }
        else{
            error(5);   /* 漏掉了分号 */
        }
    }
}

/*
 * 表达式处理
 */
void expression(bool* fsys, int* ptx, int lev){
    bool nxtlev[symnum];
  //  printf("expression  llll%d\n",sym);

    int i;
    if(sym != ID)	/* 表达式开头有标识符 */
    {
        bool_expr(fsys, ptx, lev);
    }
    else{
        i = position(id, *ptx);     /* 查找标识符在符号表中的位置 */
        if (i == 0)
        {
            error(7);	            /* 标识符未声明 */
        }
        else
        {
            getsym();
            if (sym == becomes)
            {
                getsym();
                //memcpy(nxtlev, fsys, sizeof(bool) * symnum);
         //       printf("bool");
                bool_expr(fsys, ptx, lev);	/* 处理赋值符号右侧表达式 */
                gen(sto, lev - table[i].level, table[i].adr);
            }
            else{
                switch (table[i].kind)
                {
                case intnum:	/* 标识符为整数 */
                    gen(lod, lev - table[i].level, table[i].adr);	/* 找到整数地址并将其值入栈 */
                    break;
                case boolean:	    /* 标识符为bool */
                    gen(lod, lev - table[i].level, table[i].adr);	/* 找到bool地址并将其值入栈 */
                    break;
                case constant:
                    gen(lit, 0, table[i].val);
                    break;
                }
                if(table[i].kind==boolean){
                    while(sym == andsym || sym == orsym ||sym==xorsym)
                    {
                        enum symbol bolop = sym;	/* 用于保存布尔运算 */
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        nxtlev[eql] = true;
                        nxtlev[neq] = true;
                        nxtlev[andsym] = true;
                        nxtlev[orsym] = true;
                        nxtlev[xorsym] = true;
                        getsym();
                        factor(nxtlev, ptx, lev);
                        if(bolop == andsym)
                        {
                            gen(opr, 0, 17);	/* 生成and指令 */
                        }
                        else if(bolop==xorsym)
                        {
                            gen(opr, 0, 20);    /*生成xor指令*/
                        }
                        else if(bolop==orsym)
                        {
                            gen(opr, 0, 18);	/* 生成or指令 */
                        }
                    }
                    if (sym == eql || sym == neq )
                    {
                        enum symbol  bolop = sym;
                        getsym();
                        additive_expr(fsys, ptx, lev);
                        switch (bolop)
                        {
                        case eql:
                            gen(opr, 0, 8);
                            break;
                        case neq:
                            gen(opr, 0, 9);
                            break;
                        }
                    }
                    // else{
                    // 	error(26); /*布尔变量不能用整形变量的运算符*/
                    // }
                }
                else{
                    while(sym == times || sym == slash ||sym == mod)
                {
                    enum symbol mulop = sym;	/* 用于保存乘除法符号 */
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    nxtlev[times] = true;
                    nxtlev[slash] = true;
                    nxtlev[mod] = true;
                    nxtlev[plus] = true;
                    nxtlev[minus] = true;
                    nxtlev[lss] = true;
                    nxtlev[leq] = true;
                    nxtlev[gtr] = true;
                    nxtlev[geq] = true;
                    nxtlev[eql] = true;
                    nxtlev[neq] = true;
                    getsym();
                    factor(nxtlev, ptx, lev);
                    if(mulop == times)
                    {
                        gen(opr, 0, 4);	/* 生成乘法指令 */
                    }
                    else if(mulop == slash)
                    {
                        gen(opr, 0, 5);	/* 生成除法指令 */
                    }
                    else{
                        gen(opr,0,21); /*求余*/
                    }
                }

                while (sym == plus || sym == minus)
                {
                    enum symbol adlop = sym;
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    nxtlev[times] = true;
                    nxtlev[slash] = true;
                    nxtlev[mod] = true;
                    nxtlev[plus] = true;
                    nxtlev[minus] = true;
                    nxtlev[lss] = true;
                    nxtlev[leq] = true;
                    nxtlev[gtr] = true;
                    nxtlev[geq] = true;
                    nxtlev[eql] = true;
                    nxtlev[neq] = true;
                    getsym();
                    term(nxtlev, ptx, lev);
                    if (adlop == plus)
                    {
                        gen(opr, 0, 2);	/* 生成加法指令 */
                    }
                    else
                    {
                        gen(opr, 0, 3);	/* 生成减法指令 */
                    }
                }

                if (sym == eql || sym == neq || sym == lss || sym == leq || sym == gtr || sym == geq)
                {
                    enum symbol  relop = sym;
                    getsym();
                    additive_expr(fsys, ptx, lev);
                    switch (relop)
                    {
                        case eql:
                            gen(opr, 0, 8);
                            break;
                        case neq:
                            gen(opr, 0, 9);
                            break;
                        case lss:
                            gen(opr, 0, 10);
                            break;
                        case geq:
                            gen(opr, 0, 11);
                            break;
                        case gtr:
                            gen(opr, 0, 12);
                            break;
                        case leq:
                            gen(opr, 0, 13);
                            break;
                    }
                }
                }
            }
        }
    }
}

/*
 * 布尔表达式
 */
void bool_expr(bool* fsys, int* ptx, int lev)
{
  //  printf("bool_expr");
    enum symbol relop;
    bool nxtlev[symnum];

    if(sym == oddsym)	/* 准备按照odd运算处理 */
    {
        getsym();
        expression(fsys, ptx, lev);
        gen(opr, 0, 6);	/* 生成odd指令 */
    }
    else
    {
        /* 逻辑表达式处理 */
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[eql] = true;
        nxtlev[neq] = true;
        nxtlev[lss] = true;
        nxtlev[leq] = true;
        nxtlev[gtr] = true;
        nxtlev[geq] = true;
        additive_expr(nxtlev, ptx, lev);
        if (sym == eql || sym == neq || sym == lss || sym == leq || sym == gtr || sym == geq)
        {
            relop = sym;
            getsym();
            additive_expr(fsys, ptx, lev);
            //这里运算符指令的操作要等另一个操作数加载完以后之后才能做
            switch (relop)
            {
                case eql:
                    gen(opr, 0, 8);
                    break;
                case neq:
                    gen(opr, 0, 9);
                    break;
                case lss:
                    gen(opr, 0, 10);
                    break;
                case geq:
                    gen(opr, 0, 11);
                    break;
                case gtr:
                    gen(opr, 0, 12);
                    break;
                case leq:
                    gen(opr, 0, 13);
                    break;
            }
        }
        // else
        // {
        // 	error(20); /* 应该为关系运算符 */
        // }
    }
}

void additive_expr(bool* fsys, int* ptx, int lev)
{
  //  printf("additive");
    enum symbol addop;	/* 用于保存正负号 */
    bool nxtlev[symnum];


        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        term(nxtlev, ptx, lev);	/* 处理项 */

    while (sym == plus || sym == minus)
    {
        addop = sym;
        getsym();
        // memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        // nxtlev[plus] = true;
        // nxtlev[minus] = true;
        term(nxtlev, ptx, lev);	/* 处理项 */
        if (addop == plus)
        {
            gen(opr, 0, 2);	/* 生成加法指令 */
        }
        else
        {
            gen(opr, 0, 3);	/* 生成减法指令 */
        }
    }
}

/*
 * 项处理
 */
void term(bool* fsys, int* ptx, int lev)
{
   // printf("term");
    enum symbol mulop;	/* 用于保存乘除法符号 */
    bool nxtlev[symnum];   // 后继符号集合


        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[times] = true;
        nxtlev[slash] = true;
        nxtlev[mod] = true;
        nxtlev[orsym] = true;
        nxtlev[andsym] = true;
        nxtlev[xorsym] = true;

        factor(nxtlev, ptx, lev);	/* 处理因子 */

    while(sym == times || sym == slash ||sym==mod)
    {
        mulop = sym;
        getsym();
       // printf("\nterm%d\n",sym);

        factor(nxtlev, ptx, lev);
        if(mulop == times)
        {
            gen(opr, 0, 4);	/* 生成乘法指令 */
        }
        else if(mulop == slash)
        {
            gen(opr, 0, 5);	/* 生成除法指令 */
        }
        else{
            gen(opr,0,21); /*求余*/
        }
    }
    while(sym == andsym || sym == orsym || sym==xorsym)
    {
        mulop = sym;
        getsym();
       // printf("\nterm%d\n",sym);

        factor(nxtlev, ptx, lev);
        if(mulop == andsym)
        {
            gen(opr, 0, 17);	/* 生成乘法指令 */
        }
        else if(mulop == xorsym)
        {
            gen(opr, 0, 20);
        }
        else if (mulop == orsym)
        {
            gen(opr, 0, 18);	/* 生成除法指令 */
        }

    }
}

/*
 * 因子处理
 */
void factor(bool* fsys, int* ptx, int lev)
{
  //  printf("factor");
    int i;
    bool nxtlev[symnum];
    test(facbegsys, fsys, 24);	/* 检测因子的开始符号 */
    while(inset(sym, facbegsys)) 	/* 循环处理因子 */
    {
        if(sym == ID)	/* 因子为常量或变量 */
        {
            i = position(id, *ptx);	/* 查找标识符在符号表中的位置 */
            if (i == 0)
            {
                error(7);	/* 标识符未声明 */
            }
            else
            {
                switch (table[i].kind)
                {
                    case constant:	/* 标识符为常量 */
                        gen(lit, 0, table[i].val);	/* 直接把常量的值入栈 */
                        break;
                    case intnum:	/* 标识符为int */
                        gen(lod, lev-table[i].level, table[i].adr);	/* 找到变量地址并将其值入栈 */
                        break;
                    case boolean:	/* 标识符为布尔 */
                        gen(lod, lev - table[i].level, table[i].adr);	/* 找到bool地址并将其值入栈 */
                        break;
                }
            }
            getsym();
          //  printf("\nfactor%d\n",sym);
        }
        else if(sym == notsym)	/* 表达式开头有正负号，此时当前表达式被看作一个正的或负的项 */
        {
            //addop = sym;	/* 保存开头的正负号 */
            getsym();
            if(sym == ID)	/* 因子为常量或变量 */
            {
                i = position(id, *ptx);	/* 查找标识符在符号表中的位置 */
                if (i == 0)
                {
                    error(7);	/* 标识符未声明 */
                }
                else
                {
                    if(table[i].kind==boolean){
                        gen(lod, lev - table[i].level, table[i].adr);	/* 找到bool地址并将其值入栈 */
                        gen(opr,0,19);	/* 如果开头为负号生成取负指令 */
                    }
                    else{
                        error(26); /*布尔变量不能用整形变量的运算符*/
                    }

                }
                getsym();
            }
            else if (sym == lparen)	/* 因子为表达式 */
                {
                    getsym();
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    nxtlev[rparen] = true;
                    expression(nxtlev, ptx, lev);
                    if (sym == rparen)
                    {
                        getsym();
                    }
                    else
                    {
                        error(12);	/* 缺少右括号 */
                    }
                }
            else{
                error(33);   /*不能做not操作*/
            }
        }

        else
        {
            if(sym == NUM)	/* 因子为数 */
            {
                if (num > amax)
                {
                    error(31); /* 数越界 */
                    num = 0;
                }
                gen(lit, 0, num);
                getsym();
            }
            else
            {
                if (sym == lparen)	/* 因子为表达式 */
                {
                    getsym();
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    nxtlev[rparen] = true;
                    expression(nxtlev, ptx, lev);
                    if (sym == rparen)
                    {
                        getsym();
                    }
                    else
                    {
                        error(12);	/* 缺少右括号 */
                    }
                }
                else {
                    if(sym == truesym){
                        gen(lit, 0, true);
                        getsym();
                    }
                    else if(sym == falsesym){
                        gen(lit, 0, false);
                        getsym();
                    }
                }
            }
        }
        memset(nxtlev, 0, sizeof(bool) * symnum);
        nxtlev[lparen] = true;
        nxtlev[notsym] == true;
        test(fsys, nxtlev, 23); /* 一个因子处理完毕，遇到的单词应在fsys集合中 */
                                /* 如果不是，报错并找到下一个因子的开始，使语法分析可以继续运行下去 */
      //  printf("\nfactor end %d\n",sym);
    }
}

/*
 * 解释程序
 */
bool interpret(int step,int sumstep)
{
    bool finish=false;
    FILE* fsresult= fopen("fsresult.txt", "w");
    int p = 0; /* 指令指针 */
    int b = 1; /* 指令基址 */
    int t = 0; /* 栈顶指针 */
    struct instruction i;	/* 存放当前指令 */
    int s[stacksize];	/* 栈 */

    printf("Start smallC\n");
    //fprintf(fresult,"Start smallC\n");
    s[0] = 0; /* s[0]不用 */
    s[1] = 0; /* 主程序的三个联系单元均置为0 */
    s[2] = 0;
    s[3] = 0;

    int nowstep=0;
    do {
        printf("step%dnowstep%d",step,nowstep);
        nowstep++;
        i = code[p];	/* 读当前指令 */
        p = p + 1;
        switch (i.f)
        {
            case lit:	/* 将常量a的值取到栈顶 */
                t = t + 1;
                s[t] = i.a;
                break;
            case opr:	/* 数学、逻辑运算 */
                switch (i.a)
                {
                    case 0:  /* 函数调用结束后返回 */
                        t = b - 1;
                        p = s[t + 3];
                        b = s[t + 2];
                        break;
                    case 1: /* 栈顶元素取反 */
                        s[t] = - s[t];
                        break;
                    case 2: /* 次栈顶项加上栈顶项，退两个栈元素，相加值进栈 */
                        t = t - 1;
                        s[t] = s[t] + s[t + 1];
                        break;
                    case 3:/* 次栈顶项减去栈顶项 */
                        t = t - 1;
                        s[t] = s[t] - s[t + 1];
                        break;
                    case 4:/* 次栈顶项乘以栈顶项 */
                        t = t - 1;
                        s[t] = s[t] * s[t + 1];
                        break;
                    case 5:/* 次栈顶项除以栈顶项 */
                        t = t - 1;
                        s[t] = s[t] / s[t + 1];
                        break;
                    case 6:/* 栈顶元素的奇偶判断 */
                        s[t] = s[t] % 2;
                        break;
                    case 8:/* 次栈顶项与栈顶项是否相等 */
                        t = t - 1;
                        s[t] = (s[t] == s[t + 1]);
                        break;
                    case 9:/* 次栈顶项与栈顶项是否不等 */
                        t = t - 1;
                        s[t] = (s[t] != s[t + 1]);
                        break;
                    case 10:/* 次栈顶项是否小于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] < s[t + 1]);
                        break;
                    case 11:/* 次栈顶项是否大于等于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] >= s[t + 1]);
                        break;
                    case 12:/* 次栈顶项是否大于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] > s[t + 1]);
                        break;
                    case 13: /* 次栈顶项是否小于等于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] <= s[t + 1]);
                        break;
                    case 14:/* 栈顶值输出 */
                        printf("%d", s[t]);
                        fprintf(fresult, "%d", s[t]);
                        fprintf(fsresult, "%d", s[t]);
                        t = t - 1;
                        break;
                    case 15:/* 输出换行符 */
                        printf("\n");
                        fprintf(fresult,"\n");
                        fprintf(fsresult, "\n");
                        break;
                    case 16:/* 读入一个输入置于栈顶 */
                        t = t + 1;
                        printf("?");
                        fprintf(fresult, "?");
                        fprintf(fsresult, "?");
                        fscanf(fsinput,"%d", &(s[t]));
                        fprintf(fresult, "%d\n", s[t]);
                        fprintf(fsresult, "%d\n", s[t]);
                        break;
                    case 17:     /*and操作*/
                        t = t - 1;
                        s[t] = s[t]*s[ t + 1 ];
                        break;
                    case 18:
                        t = t - 1;  /* or 操作*/
                        if(s[t]== 1|| s[t+1]==1){
                            s[t] = 1;
                        }
                        else{
                            s[t] = 0;
                        }
                        break;
                    case 19:    /*not 操作*/
                        //t = t - 1;   //这里必须先出栈，再做运算
                        s[t] = 1 - s[t];
                        break;

                    case 20:    /*xor*/
                        t = t - 1;
                        if(s[t]==s[t+1]){
                            s[t]=0;
                        }
                        else{
                            s[t]=1;
                        }
                        break;
                    case 21:   /*求余*/
                        t = t-1;
                        s[t] = (s[t] % s[t + 1]);
                        break;

                }
                //		输出数据栈
                // for(int i = 0;i<=t+1;i++){
                // 	printf("%d",s[i]);
                // }
                // printf("\n");
                break;
            case lod:	/* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
                t = t + 1;
                s[t] = s[base(i.l,s,b) + i.a];
                break;
            case sto:	/* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
                s[base(i.l, s, b) + i.a] = s[t];
                t = t - 1;
                break;
            case cal:	/* 调用子过程 */
                s[t + 1] = base(i.l, s, b);	/* 将父过程基地址入栈，即建立静态链 */
                s[t + 2] = b;	/* 将本过程基地址入栈，即建立动态链 */
                s[t + 3] = p;	/* 将当前指令指针入栈，即保存返回地址 */
                b = t + 1;	/* 改变基地址指针值为新过程的基地址 */
                p = i.a;	/* 跳转 */
                break;
            case ini:	/* 在数据栈中为被调用的过程开辟a个单元的数据区 */
                t = t + i.a;
                break;
            case jmp:	/* 直接跳转 */
                p = i.a;
                break;
            case jpc:	/* 条件跳转 */
                if (s[t] == 0)
                    p = i.a;
                t = t - 1;
                break;
        }
    } while (p != 0 && (step==0||nowstep<step));

    fflush(fsresult);

    fstack = fopen("fstack.txt", "w");
    FILE* fscode = fopen("fscode.txt", "w");
    for(int j = 1; j <= t; j++) {
        fprintf(fstack, "s[%d]------%d\n", j, s[j]);
        fflush(fstack);
    }

    printf("listswitch%d\n",listswitch);

        for (int j = 0; j < cx; j++)
        {
            printf("%d %s %d %d\n", j, mnemonic[code[j].f], code[j].l, code[j].a);
            fflush(stdout);
            fprintf(fscode, "%d %s %d %d", j, mnemonic[code[j].f], code[j].l, code[j].a);
            fflush(fscode);
            if(p-1 == j) {
                fprintf(fscode, "  <--");
            }
            fprintf(fscode, "\n");
        }
                    if(step>0 && p==sumstep){
                        finish=true;
                    }

    //fprintf(fscode, "%d\n", p);
    fflush(fscode);

    fclose(fstack);
    fclose(fscode);
    fclose(fsresult);

    printf("End smallC\n");
    //fprintf(fresult,"End smallC\n");
    if(finish){
        return true;
    }
    else
        return false;
}

/* 通过过程基址求上l层过程的基址 */
int base(int l, int* s, int b)                 /* l是层次差 */
{
    int b1;
    b1 = b;
    while (l > 0)
    {
        b1 = s[b1];
        l--;
    }
    return b1;
}

