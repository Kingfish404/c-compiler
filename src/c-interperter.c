#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define int long long

char token;             // current token;
char *src, *old_src;    // pointer to source code string;
int pool_size;          // default size of text/data/stack;
int line;               // line number;

// virtual machine data;
int *text;            // text segment
int *old_text;        // for dump text segment
int *stack;           // stack
char *data;           // data segment

// virtual machine registers
// pc - program counter - 程序计数器，它存放的是一个内存地址，该地址中存放着 下一条 要执行的计算机指令。
int *pc;
// bp - basic pointer - 基址指针。也是用于指向栈的某些位置，在调用函数时会使用到它。
int *bp;
// sp - stack pointer - 指针寄存器，指向当前的栈顶
int *sp;
// ax - accumulator register - 通用寄存器，存放指令结果
int ax;
int cycle;
//    +------------------+
//    |    stack   |     |      high address
//    |    ...     v     |
//    |                  |
//    |                  |
//    |                  |
//    |                  |
//    |    ...     ^     |
//    |    heap    |     |
//    +------------------+
//    | bss  segment     |
//    +------------------+
//    | data segment     |
//    +------------------+
//    | text segment     |      low address
//    +------------------+

// instructions
enum
{
    LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH,
    OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};

// tokens and classes (operators last and in precedence order)
enum
{
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// identifier for p;
struct identifier
{
    int token;      // 该标识符返回的标记./id
    int hash;       //
    char *name;    // 标识符本身字符串
    int class;      // 类别，number，全局，局部
    int type;       // 类型，int/char
    int value;      // 值
    int Bclass;     // 扩展标识，用于区分局部还是全局
    int Btype;
    int Bvalue;
};

int token_val;                // value of current token (mainly for number)
int *current_id;              // current parsed ID
int *symbols;                 // symbol table
int *idmain;                  // the `main` function

// fields of identifier
enum
{
    Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize
};

// types of variable/function
enum
{
    CHAR, INT, PTR
};

void next()
{
    char *last_pos;
    int hash;

    while (token = *src)
    {
        ++src;

        // parse token here
        if (token == '\n')
        {
            ++line;
        } else if (token == '#')
        {
            // skip macro, because we will not support it
            while (*src != 0 && *src != '\n')
            {
                src++;
            }
        } else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_'))
        {

            // parse identifier
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') ||
                   (*src == '_'))
            {
                hash = hash * 147 + *src;
                src++;
            }

            // look for existing identifier, linear search
            current_id = symbols;
            while (current_id[Token])
            {
                if (current_id[Hash] == hash && !memcmp((char *) current_id[Name], last_pos, src - last_pos))
                {
                    //found one, return
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }


            // store new ID
            current_id[Name] = (int) last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        } else if (token >= '0' && token <= '9')
        {
            // parse number, three kinds: dec(123) hex(0x123) oct(017)
            token_val = token - '0';
            if (token_val > 0)
            {
                // dec, starts with [1-9]
                while (*src >= '0' && *src <= '9')
                {
                    token_val = token_val * 10 + *src++ - '0';
                }
            } else
            {
                // starts with 0
                if (*src == 'x' || *src == 'X')
                {
                    //hex
                    token = *++src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') ||
                           (token >= 'A' && token <= 'F'))
                    {
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *++src;
                    }
                } else
                {
                    // oct
                    while (*src >= '0' && *src <= '7')
                    {
                        token_val = token_val * 8 + *src++ - '0';
                    }
                }
            }

            token = Num;
            return;
        } else if (token == '"' || token == '\'')
        {
            // parse string literal, currently, the only supported escape
            // character is '\n', store the string literal into data.
            last_pos = data;
            while (*src != 0 && *src != token)
            {
                token_val = *src++;
                if (token_val == '\\')
                {
                    // escape character
                    token_val = *src++;
                    if (token_val == 'n')
                    {
                        token_val = '\n';
                    }
                }

                if (token == '"')
                {
                    *data++ = token_val;
                }
            }

            src++;
            // if it is a single character, return Num token
            if (token == '"')
            {
                token_val = (int) last_pos;
            } else
            {
                token = Num;
            }

            return;
        } else if (token == '/')
        {
            if (*src == '/')
            {
                // skip comments
                while (*src != 0 && *src != '\n')
                {
                    ++src;
                }
            } else
            {
                // divide operator
                token = Div;
                return;
            }
        } else if (token == '=')
        {
            // parse '==' and '='
            if (*src == '=')
            {
                src++;
                token = Eq;
            } else
            {
                token = Assign;
            }
            return;
        } else if (token == '+')
        {
            // parse '+' and '++'
            if (*src == '+')
            {
                src++;
                token = Inc;
            } else
            {
                token = Add;
            }
            return;
        } else if (token == '-')
        {
            // parse '-' and '--'
            if (*src == '-')
            {
                src++;
                token = Dec;
            } else
            {
                token = Sub;
            }
            return;
        } else if (token == '!')
        {
            // parse '!='
            if (*src == '=')
            {
                src++;
                token = Ne;
            }
            return;
        } else if (token == '<')
        {
            // parse '<=', '<<' or '<'
            if (*src == '=')
            {
                src++;
                token = Le;
            } else if (*src == '<')
            {
                src++;
                token = Shl;
            } else
            {
                token = Lt;
            }
            return;
        } else if (token == '>')
        {
            // parse '>=', '>>' or '>'
            if (*src == '=')
            {
                src++;
                token = Ge;
            } else if (*src == '>')
            {
                src++;
                token = Shr;
            } else
            {
                token = Gt;
            }
            return;
        } else if (token == '|')
        {
            // parse '|' or '||'
            if (*src == '|')
            {
                src++;
                token = Lor;
            } else
            {
                token = Or;
            }
            return;
        } else if (token == '&')
        {
            // parse '&' and '&&'
            if (*src == '&')
            {
                src++;
                token = Lan;
            } else
            {
                token = And;
            }
            return;
        } else if (token == '^')
        {
            token = Xor;
            return;
        } else if (token == '%')
        {
            token = Mod;
            return;
        } else if (token == '*')
        {
            token = Mul;
            return;
        } else if (token == '[')
        {
            token = Brak;
            return;
        } else if (token == '?')
        {
            token = Cond;
            return;
        } else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' ||
                   token == ']' || token == ',' || token == ':')
        {
            // directly return the character as token;
            return;
        }
    }
    return;
}


void expression(int level)
{
    // TODO: do nothing,need to write
}

void program()
{
    next();             // get next token;
    while (token > 0)
    {
        printf("token is: %c\n", token);
        next();
    }
}

int eval()
{
    int op, *tmp;
    while (1)
    {
        op = *pc++;   // get next operation code
        switch (op)
        {
            case IMM:   // load immediate value to ax
                ax = *pc;
                pc++;
                break;
            case LC:    // load character to ax, address in ax
                ax = *(char *) ax;
                break;
            case LI:    // load integer to ax, address in ax
                ax = *(int *) ax;
                break;
            case SC:    // save character to address, value in ax, address on stack
                ax = *(char *) *sp++ = ax;
                break;
            case SI:    // save integer to address, value in ax, address on stack
                *(int *) *sp++ = ax;
                break;
            case PUSH:  // push the value of ax onto the stack
                sp--;
                *sp = ax;
                break;
            case JMP:   // jump to the address
                pc = (int *) *pc;
                break;
            case JZ:    // jump if ax is zero
                pc = ax ? pc + 1 : (int *) *pc;
                break;
            case JNZ:   // jump if ax is not zero
                pc = ax ? (int *) *pc : pc + 1;
                break;
            case CALL:  // call subroutine
                *--sp = (int) (pc + 1);
                pc = (int *) *pc;
                break;
            case ENT:   // make new stack frame
                *--sp = (int) bp;
                bp = sp;
                sp = sp - *pc++;
                break;
            case ADJ:   // add esp, <size>
                sp = sp + *pc++;
                break;
            case LEV:   // restore call frame and PC
                sp = bp;
                bp = (int *) *sp++;
                pc = (int *) *sp++;
                break;
            case LEA:   // load address for arguments.
                ax = (int) (bp + *pc++);
                break;
            default:
                // 运算符
                if (op == OR) ax = *sp++ | ax;
                else if (op == XOR) ax = *sp++ ^ ax;
                else if (op == AND) ax = *sp++ & ax;
                else if (op == EQ) ax = *sp++ == ax;
                else if (op == NE) ax = *sp++ != ax;
                else if (op == LT) ax = *sp++ < ax;
                else if (op == LE) ax = *sp++ <= ax;
                else if (op == GT) ax = *sp++ > ax;
                else if (op == GE) ax = *sp++ >= ax;
                else if (op == SHL) ax = *sp++ << ax;
                else if (op == SHR) ax = *sp++ >> ax;
                else if (op == ADD) ax = *sp++ + ax;
                else if (op == SUB) ax = *sp++ - ax;
                else if (op == MUL) ax = *sp++ * ax;
                else if (op == DIV) ax = *sp++ / ax;
                else if (op == MOD) ax = *sp++ % ax;
                else if (op == EXIT)
                {
                    printf("exit(%lld)", *sp);
                    return *sp;
                }
                    // TODO: fix this 3 item
                else if (op == OPEN)
                {
//                    ax = fopen((char *) sp[1], sp[0]);
                    printf("open not support!\n");
                } else if (op == CLOS)
                {
//                    ax = fclose(*sp);
                    printf("close not support!\n");
                } else if (op == READ)
                {
//                    ax = fread(sp[2], (char *)sp[1], *sp);
                    printf("read not support!\n");
                } else if (op == PRTF)
                {
                    tmp = sp + pc[1];
                    ax = printf((char *) tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
                } else if (op == MALC)
                { ax = (int) malloc(*sp); }
                else if (op == MSET)
                { ax = (int) memset((char *) sp[2], sp[1], *sp); }
                else if (op == MCMP)
                { ax = memcmp((char *) sp[2], (char *) sp[1], *sp); }
                else
                {
                    printf("unknown instruction:%lld\n", op);
                    return -1;
                }
                break;
        }
    }
    return 0;
}

int allocateMemory()
{
    pool_size = 256 * 1024;    // arbitrary size;

    // allocate memory for virtual machine;
    if (!(text = old_text = malloc(pool_size)))
    {
        printf("could not malloc(%lld) for text area\n", pool_size);
        return -1;
    }
    if (!(data = malloc(pool_size)))
    {
        printf("could not malloc(%lld) for data area\n", pool_size);
        return -1;
    }
    if (!(stack = malloc(pool_size)))
    {
        printf("could not malloc(%lld) for stack area\n", pool_size);
        return -1;
    }
    if (!(symbols = malloc(pool_size)))
    {
        printf("could not malloc(%lld) for symbol table\n", pool_size);
        return -1;
    }

    // init virtual machine data;
    memset(text, 0, pool_size);
    memset(data, 0, pool_size);
    memset(stack, 0, pool_size);
    memset(symbols, 0, pool_size);

    bp = sp = (int *) ((int) stack + pool_size);
    ax = 0;

    return 0;
}

int runCode(int argc, char **argv)
{
    int i;
    FILE *fd;

    argc--;
    argv++;

    line = 1;

    // allocateMemory
    if (allocateMemory())
    {
        return -1;
    }

    src = "char else enum if int return sizeof while "
          "open read close printf malloc memset memcmp exit void main";

    // add keywords to symbol table
    i = Char;
    while (i <= While)
    {
        next();
        current_id[Token] = i++;
    }

    // add library to symbol table
    i = OPEN;
    while (i <= EXIT)
    {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }

    next();
    current_id[Token] = Char; // handle void type
    next();
    idmain = current_id; // keep track of main

    // file and safe check start;
    fd = fopen(*argv, "r");
    if (!fd)
    {
        printf("could not open(%s)\n", *argv);
        return -1;
    }
    // file and safe check end;
    // read the source file;
    if (!(src = old_src = malloc(pool_size)))
    {
        printf("could not malloc(%lld) for source area\n", pool_size);
        return -1;
    }
    if ((i = fread(src, sizeof(char), pool_size - 1, fd)) < 0)
    {
        printf("read() returned %lu\n", i);
        return -1;
    }

//    src[i] = 0;
    fclose(fd);

    // set pc at the start of code
    pc = text;

    // program;
    program();

    // run code and return;
    return eval();
}

int testEval()
{
    unsigned long i;
    // allocateMemory
    if (allocateMemory())
    {
        return -1;
    }

    bp = sp = (int *) ((int) stack + pool_size);
    ax = 0;

    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;

    // program;
    program();

    // run code and return;
    return eval();
}

#undef int

int main(int argc, char **argv)
{
    // test eval
//    testEval();

    // run code from arguments
    runCode(argc, argv);
    return 0;
}