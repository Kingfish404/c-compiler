// -*- C++ -*-
// @file regex2nfa.cpp
// @describe this cpp file is use to translate the regular expression to NFA
// @描述 此文件用于识别匹配指定字符串是否符合正则
// @author Jin Yu
// @date 2020-11-18
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <utility>
#include <vector>
#include <map>
#include <cstdio>
#include <set>
#include <queue>
#include <algorithm>
#include <iterator>
#include <sstream>

#define noEdge -1       // dfa simplify 宏变量

#define DIV_CHAR '+'    // 这是用来分隔的字符，表示连接

bool CHECK_ON = false;  // 是否开启检查内容的输入输出

namespace regex2nfa
{
    class State
    {
    public:
        int ID{};
        std::map<char, State *> transitions;
        std::vector<State *> transitions_e;
        bool isEnd{};
    public:
        State() = default;

        explicit State(int ID)
        {
            this->ID = ID;
        }

        State(int ID, bool isEnd)
        {
            this->ID = ID;
            this->isEnd = isEnd;
        }
    };

    class Fragment
    {
    public:
        State *state_start;
        State *state_end;
    public:
        Fragment() = default;

        Fragment(State *start, State *end)
        {
            this->state_start = start;
            this->state_end = end;
        }
    };

    class NFA
    {
    public:
        std::vector<State *> states;

    public:
        NFA() = default;

        void removeState(const State *state)
        {
            for (auto it = this->states.begin(); it != this->states.end(); it++)
            {
                if ((*it)->ID == state->ID)
                {
                    State *p = *it;
                    this->states.erase(it);
                    if (p != nullptr)
                    {
                        delete p;
                        break;
                    }
                }
            }
        }

        ~NFA()
        {
            for (const State *i : states)
            {
                delete i;
            }
        }
    };

    class Regex2Nfa
    {
    private:
        const char explicitC = DIV_CHAR;
        std::string input;          // 原始的输入字符串
        std::string strBeInsert;    // 插入分隔符后的字符串
        std::string strPostfix;
        std::string alphaBet;       // 字母表
        std::map<char, int> op_priority;  // 运算符优先级
        NFA nfa;

        // 输入输出
    public:
        Regex2Nfa()
        {
            alphaBet = std::string();
            char tempChar;
            // init alphaBet as [A,B,C,...,a,b,c,...,0,1,2,...]
            for (int i = 'A'; i < 'Z' + 1; i++)
            {
                tempChar = char(i);
                alphaBet.push_back(tempChar);
                tempChar = char(i + 'a' - 'A');
                alphaBet.push_back(tempChar);
            }
            for (int i = '0'; i < '9' + 1; i++)
            {
                tempChar = char(i);
                alphaBet.push_back(tempChar);
            }
            op_priority['*'] = 4;
            op_priority['|'] = 3;
            op_priority[explicitC] = 2;
            op_priority['('] = 0;
        }

        void getInput()
        {
            std::cin >> input;
        }

        void setInput(const std::string &inputT)
        {
            input = inputT;
        }

        void outputOrigin()
        {
            for (const State *state : nfa.states)
            {
                if (state == nfa.states.back())
                {
                    continue;
                }
                std::cout << state->ID << " ";

                for (State *const state_item : state->transitions_e)
                {
                    std::cout << state->ID << "-~->" << state_item->ID << " ";
                }

                for (std::pair<const char, State *> pairTemp : state->transitions)
                {
                    std::cout << state->ID << "-" <<
                              pairTemp.first << "->" << pairTemp.second->ID << " ";
                }
                std::cout << std::endl;
            }
        }

        void output()
        {
            for (const State *state : nfa.states)
            {
                if (state == nfa.states.front())
                {
                    std::cout << "X ";
                    for (State *const state_item : state->transitions_e)
                    {
                        if (state_item->ID == nfa.states.back()->ID)
                        {
                            std::cout << "X-~->Y ";
                        } else
                        {
                            std::cout << "X-~->" << state_item->ID << " ";
                        }
                    }
                    for (std::pair<const char, State *> pairTemp : state->transitions)
                    {
                        if (pairTemp.second->ID == nfa.states.back()->ID)
                        {
                            std::cout << "X-" <<
                                      pairTemp.first << "->Y ";
                        } else
                        {
                            std::cout << "X-" <<
                                      pairTemp.first << "->" << pairTemp.second->ID << " ";
                        }
                    }
                    std::cout << std::endl << "Y" << std::endl;
                } else if (state == nfa.states.back())
                {
                    continue;
                } else
                {
                    std::cout << state->ID << " ";
                    for (State *const state_item : state->transitions_e)
                    {
                        if (state_item->ID == nfa.states.back()->ID)
                        {
                            std::cout << state->ID << "-" <<
                                      '~' << "->Y ";
                        } else
                        {
                            std::cout << state->ID << "-" <<
                                      '~' << "->" << state_item->ID << " ";
                        }
                    }

                    for (std::pair<const char, State *> pairTemp : state->transitions)
                    {

                        if (pairTemp.second->ID == nfa.states.back()->ID)
                        {
                            std::cout << state->ID << "-" <<
                                      pairTemp.first << "->" << "Y";
                        } else
                        {
                            std::cout << state->ID << "-" <<
                                      pairTemp.first << "->" << pairTemp.second->ID << " ";
                        }
                    }
                    std::cout << std::endl;
                }

            }
        }

        void outputToFile()
        {
            std::streambuf *coutBuf = std::cout.rdbuf();
            std::ofstream of("tmp_nfa_orign.txt");
            std::streambuf *fileBuf = of.rdbuf();
            std::cout.rdbuf(fileBuf);

            for (const State *state : nfa.states)
            {
                // 对于第一个X的输出
                if (state == nfa.states.front())
                {
                    int countE = 0, countEdge = 0;
                    for (State *const state_item : state->transitions_e)
                    {
                        countE++;
                    }
                    for (std::pair<const char, State *> pairTemp : state->transitions)
                    {
                        countEdge++;
                    }
                    std::cout << "X ";
                    for (State *const state_item : state->transitions_e)
                    {
                        countE--;
                        if (countE == 0 && countEdge == 0)
                        {
                            if (state_item->ID == nfa.states.back()->ID)
                            {
                                std::cout << "X-~->Y";
                            } else
                            {
                                std::cout << "X-~->" << state_item->ID;
                            }
                        } else
                        {
                            if (state_item->ID == nfa.states.back()->ID)
                            {
                                std::cout << "X-~->Y";
                            } else
                            {
                                std::cout << "X-~->" << state_item->ID << " ";
                            }
                        }
                    }
                    for (std::pair<const char, State *> pairTemp : state->transitions)
                    {
                        countEdge--;
                        if (countEdge == 0)
                        {
                            if (pairTemp.second->ID == nfa.states.back()->ID)
                            {
                                std::cout << "X-" <<
                                          pairTemp.first << "->Y";
                            } else
                            {
                                std::cout << "X-" <<
                                          pairTemp.first << "->" << pairTemp.second->ID;
                            }
                        } else
                        {
                            if (pairTemp.second->ID == nfa.states.back()->ID)
                            {
                                std::cout << "X-" <<
                                          pairTemp.first << "->Y";
                            } else
                            {
                                std::cout << "X-" <<
                                          pairTemp.first << "->" << pairTemp.second->ID << " ";
                            }
                        }
                    }
                    std::cout << std::endl << "Y " << std::endl;
                } else if (state == nfa.states.back())
                {
                    continue;
                } else
                {
                    // 对于后面的
                    int countE = 0, countEdge = 0;
                    for (State *const state_item : state->transitions_e)
                    {
                        countE++;
                    }
                    for (std::pair<const char, State *> pairTemp : state->transitions)
                    {
                        countEdge++;
                    }
                    std::cout << state->ID << " ";
                    for (State *const state_item : state->transitions_e)
                    {
                        countE--;
                        if (countE == 0 && countEdge == 0)
                        {
                            if (state_item->ID == nfa.states.back()->ID)
                            {
                                std::cout << state->ID << "-" <<
                                          '~' << "->Y ";
                            } else
                            {
                                std::cout << state->ID << "-" <<
                                          '~' << "->" << state_item->ID;
                            }
                        } else
                        {
                            if (state_item->ID == nfa.states.back()->ID)
                            {
                                std::cout << state->ID << "-" <<
                                          '~' << "->Y ";
                            } else
                            {
                                std::cout << state->ID << "-" <<
                                          '~' << "->" << state_item->ID << " ";
                            }
                        }
                    }

                    for (std::pair<const char, State *> pairTemp : state->transitions)
                    {
                        countEdge--;
                        if (countEdge == 0)
                        {
                            if (pairTemp.second->ID == nfa.states.back()->ID)
                            {
                                std::cout << state->ID << "-" <<
                                          pairTemp.first << "->" << "Y";
                            } else
                            {
                                std::cout << state->ID << "-" <<
                                          pairTemp.first << "->" << pairTemp.second->ID;
                            }
                        } else
                        {
                            if (pairTemp.second->ID == nfa.states.back()->ID)
                            {
                                std::cout << state->ID << "-" <<
                                          pairTemp.first << "->" << "Y";
                            } else
                            {
                                std::cout << state->ID << "-" <<
                                          pairTemp.first << "->" << pairTemp.second->ID << " ";
                            }
                        }
                    }
                    std::cout << std::endl;
                }

            }

            of.flush();
            of.close();

            std::cout.rdbuf(coutBuf);
        }

    public:
        // 向正则表达式中插入分隔符号
        void insertExplicit()
        {
            strBeInsert = input;
            bool shouldInsert = false;
            int count = 0;  // 用于标记当前扫描到的字符
            for (unsigned int i = 0; i < input.length() - 1; i++, count++)
            {
                // 对于如ab,a(c),a*(c),(a)b,a*b,五种状态
                // 替换成a_b,a_(c),a*_(c),(a)_b,a*_b
                if (isCharInStr(alphaBet, strBeInsert[count]) &&
                    (isCharInStr(alphaBet, strBeInsert[count + 1]) || strBeInsert[count + 1] == '('))
                {
                    // 对于两个字母相邻，插入分隔
                    shouldInsert = true;
                } else if (strBeInsert[count] == '*' &&
                           strBeInsert[count + 1] == '(')
                {
                    // 对于字母或者'*'与'('相邻，插入分隔
                    shouldInsert = true;
                } else if (strBeInsert[count] == ')' && isCharInStr(alphaBet, strBeInsert[count + 1]))
                {
                    // 对于')'和字母相邻，插入分隔
                    shouldInsert = true;
                } else if (strBeInsert[count] == '*' && isCharInStr(alphaBet, strBeInsert[count + 1]))
                {
                    // 对于*和字母相邻，插入分隔
                    shouldInsert = true;
                }

                if (shouldInsert)
                {
                    strBeInsert.insert(count + 1, std::string(1, explicitC));
                    count++;
                    shouldInsert = false;
                }
            }
        }

        // 转成后缀表达式
        void convertToPostfix()
        {
            std::string outputPostfix;
            std::vector<char> op_stack;
            char tempChar;

            //    转后缀表达式
            //    1.遇到操作数，直接输出；
            //    2.栈为空时，遇到运算符，入栈；
            //    3.遇到左括号，将其入栈；
            //    4.遇到右括号，执行出栈操作，并将出栈的元素输出，直到弹出栈的是左括号，左括号不输出；
            //    5.遇到其他运算符’+”-”*”/’时，弹出所有优先级大于或等于该运算符的栈顶元素，然后将该运算符入栈；
            //    6.最终将栈中的元素依次出栈，输出。
            //    举例：a+b*c+(d*e+f)g ———> abc*+de*f+g*+
            for (char ch : strBeInsert)
            {
                if (isCharInStr(alphaBet, ch))
                {
                    outputPostfix.push_back(ch);
                } else
                {
                    if (op_stack.empty() || ch == '(')
                    {
                        op_stack.push_back(ch);
                    } else if (ch == ')')
                    {
                        while (op_stack.back() != '(')
                        {
                            tempChar = op_stack.back();
                            outputPostfix.push_back(tempChar);
                            op_stack.pop_back();
                        }
                        op_stack.pop_back();
                    } else
                    {
                        while (!op_stack.empty() && op_priority[ch] <= op_priority[op_stack.back()])
                        {
                            tempChar = op_stack.back();
                            outputPostfix.push_back(tempChar);
                            op_stack.pop_back();
                        }
                        op_stack.push_back(ch);
                    }
                }
            }
            while (!op_stack.empty())
            {
                tempChar = op_stack.back();
                outputPostfix.push_back(tempChar);
                op_stack.pop_back();
            }
            strPostfix = outputPostfix;
        }

        // Using Thompson Alg to construct
        void constructToNFA()
        {
            std::vector<Fragment *> stack_frag;
            int stateId = -1;

            // temp various
            int i_t;
            State *state_start;
            State *state_end;
            Fragment *frag_start;
            Fragment *frag_end;
            Fragment *newFragment;

            for (const char ch : strPostfix)
            {
                switch (ch)
                {
                    case '*':
                        // closure 遇到*，生成多个运算
                        // From f2:s2->s3
                        // to fn:s2-e->s3,s3-a->s3,s3-e->s4
                        frag_end = stack_frag.back();
                        stack_frag.pop_back();

                        frag_end->state_start->transitions_e.push_back(frag_end->state_end);

                        for (std::pair<const char, State *> &pairTemp : frag_end->state_start->transitions)
                        {
                            pairTemp.second = frag_end->state_end;
                            frag_end->state_end->transitions.insert(pairTemp);
                        }
                        frag_end->state_start->transitions.clear();
                        frag_end->state_end->isEnd = false;

                        stateId = getMaxStateID(nfa) + 1;
                        state_end = new State(stateId, true);
                        stateId++;
                        nfa.states.push_back(state_end);

                        frag_end->state_end->transitions_e.push_back(state_end);
                        frag_end->state_end = state_end;

                        stack_frag.push_back(frag_end);
                        break;
                    case '|':
                        // union 遇到或
                        frag_end = stack_frag.back();
                        stack_frag.pop_back();
                        frag_start = stack_frag.back();
                        stack_frag.pop_back();

                        for (std::pair<const char, State *> it : frag_end->state_start->transitions)
                        {
                            it.second = frag_start->state_end;
                            frag_start->state_start->transitions.insert(it);
                        }

                        nfa.removeState(frag_end->state_start);
                        nfa.removeState(frag_end->state_end);

                        stateId = getMaxStateID(nfa) + 1;

                        newFragment = new Fragment(frag_start->state_start, frag_start->state_end);
                        stack_frag.push_back(newFragment);
                        delete frag_end;
                        delete frag_start;
                        break;
                    case DIV_CHAR:
                        // concat 直接连接两个fragment
                        // 将 f_s:s1->s2,f_e:s3->s4
                        // 变成 f_n:s1->s2->s4
                        frag_end = stack_frag.back();
                        stack_frag.pop_back();
                        frag_start = stack_frag.back();
                        stack_frag.pop_back();

                        frag_start->state_end->transitions = frag_end->state_start->transitions;
                        frag_start->state_end->transitions_e = frag_end->state_start->transitions_e;

                        // 移除frag_end中的state_start
                        nfa.removeState(frag_end->state_start);

                        frag_end->state_start = frag_start->state_end;

                        i_t = -1;
                        for (State *state : nfa.states)
                        {
                            state->ID = i_t++;
                        }

                        stateId = getMaxStateID(nfa) + 1;
                        newFragment = new Fragment(frag_start->state_start, frag_end->state_end);
                        stack_frag.push_back(newFragment);
                        delete frag_end;
                        delete frag_start;
                        break;
                    default:
                        // 遇到普通字符
                        state_start = new State(stateId, false);
                        stateId++;
                        state_end = new State(stateId, true);
                        stateId++;
                        state_start->transitions[ch] = state_end;
                        newFragment = new Fragment(state_start, state_end);

                        nfa.states.push_back(state_start);
                        nfa.states.push_back(state_end);

                        stack_frag.push_back(newFragment);
                        break;
                }
            }
        }

    private:
        // 检查一个字符是否在字符串里面
        static bool isCharInStr(const std::string &s, const char c)
        {
            std::size_t idx;
            idx = s.find(c);
            if (idx != std::string::npos)
            {
                return true;
            } else
            {
                return false;
            }
        }

        static int getMaxStateID(const NFA &nfa)
        {
            int maxState = -1;
            for (const State *state : nfa.states)
            {
                maxState = (maxState > state->ID) ? maxState : state->ID;
            }
            return maxState;
        }

        // 检查变量
    public:
        void printInputByChar()
        {
            std::cout << "This is input: ";
            for (char i : input)
            {
                std::cout << i;
            }
            std::cout << std::endl;
        }

        void printInsertStrByChar()
        {
            std::cout << "This is inserted: ";
            for (char i : strBeInsert)
            {
                std::cout << i;
            }
            std::cout << std::endl;
        }

        void printPostfixStrByChar()
        {
            printf("This is postfix: ");
            for (char i : strPostfix)
            {
                std::cout << i;
            }
            std::cout << std::endl;
        }
    };

    void regex2nfa()
    {
        std::cout << "Please enter the regex:";

        regex2nfa::Regex2Nfa regex2nfa;

        // get regex expression,eg: (a|b)*c,abed
//    regex2nfa.setInput("(a|b)*abb");
        regex2nfa.getInput();
        //    regex2nfa.setInput("abb");

        // [Treat]Insert explicit concatenation operator,(a|b)*c -> (a|b)*_c
        regex2nfa.insertExplicit();

        // [Treat]Convert to postfix notation, (a|b)*_c -> ab|*c
        regex2nfa.convertToPostfix();

        // [Treat]Construct an NFA
        regex2nfa.constructToNFA();

        if (CHECK_ON)
        {
            // [Check]check the regex expression
            regex2nfa.printInputByChar();
            // [Check]check the string after being insert
            regex2nfa.printInsertStrByChar();
            // [Check]check postfix string
            regex2nfa.printPostfixStrByChar();
        }

        // [Check]Output the final NFA

        regex2nfa.output();

        regex2nfa.outputToFile();
    };
}

namespace nfa2dfa
{
    struct trans
    {
        char start; //转换初态
        char receive; //接受的字母
        char end; //转换的结果
    };

    struct NFA
    {
        std::set<char> state; //状态集，默认X初态，Y终态不单独列出
        std::set<char> word;  //字母表 空字母~不放进去
        std::vector<struct trans> transfunc; //状态转换函数
    };

    struct DFA
    {
        std::set<char> state; //状态集，默认X初态，Y终态不单独列出
        std::set<char> word;  //字母表 空字母~不放进去
        std::vector<struct trans> transfunc; //状态转换函数
    };

    // NFA输入，NFAM用来储存

    void input(struct NFA &NFAM)
    {

        int newend = 0; //0表示没新状态产生
        char tempChar;
        int all_have_vex = 1;//1表示每个中间状态都发出了弧，也就是在输入过程中作为NFA的state出现过
        FILE *fin = fopen("tmp_nfa_orign.txt", "r");
        std::map<char, int> statemarked = {}; //value=0表示状态没有发出的弧

        char state; //状态  每行输入开头的状态
        struct trans temptrans;// 临时存储输入的转换规则

        // 输入流设为文件
//        freopen("tmp_nfa_orign.txt","r",stdin);

        //大行输入
        while (1)
        {

            fscanf(fin, "%c", &state);
            fscanf(fin, "%c", &tempChar);
            NFAM.state.insert(state);//状态插入到集合中
            //如果曾经出现过state状态，则对其标记
            if (statemarked.count(state))
                statemarked[state] = 1;
            //行内转换规则输入
            newend = 0;//0表示没新状态产生
            all_have_vex = 1;//1是默认状态，表示每个中间状态都发出了弧
            while (1)
            {

                if (state == 'Y')
                {
                    //Y的情况是转换规则接受一个空格然后换行
                    //getchar();//取走空格
                    fscanf(fin, "%c", &tempChar);
                    if (tempChar == '\n')
                    {//取换行并判断转换规则输入结束
                        break;
                    }
                    //break;
                }
                //普通输入情况
                fscanf(fin, "%c-%c->%c", &temptrans.start, &temptrans.receive, &temptrans.end);
                NFAM.word.insert(temptrans.receive);//添加字母表
                NFAM.transfunc.push_back(temptrans);//添加转换规则
                //判断有无新状态end产生
                if (NFAM.state.count(temptrans.end) == 0)
                {
                    //等于0表示之前没这个状态，代表有新状态产生
                    newend = 1;
                    //新状态添加到statemarked中 新状态仅包含0，1，2，3等，不含初态终态
                    if (temptrans.end != 'Y')
                        statemarked[temptrans.end] = 0;
                }

                fscanf(fin, "%c", &tempChar);
                if (tempChar == '\n')
                {
                    break;
                }
            }

            //看所有状态是否都发出了弧
            std::map<char, int>::iterator it;
            for (it = statemarked.begin(); it != statemarked.end(); it++)
            {
                if ((*it).second == 0)
                {
                    //0表示没发出弧
                    all_have_vex = 0;
                }
            }

            //判断有无新状态产生
            if (newend == 0 && state != 'Y')
            {
                //无新状态产生且不是终态Y
                //如果statemarked中的所有状态都发出了弧，既value都为1则break
                if (all_have_vex == 1)
                {
                    break;
                }
            }
            //getchar();//再取走一个换行
        }

    }


    //求集合的ε-closure状态子集 此集合包含I自身
    std::set<char> closure(std::set<char> I, struct NFA NFAM)
    {
        std::set<char> closureset = I;//最后的结果储存,现把自身都放进去

        //对集合I每个状态遍历，找到其所能到达的点
        for (std::set<char>::iterator it = I.begin(); it != I.end(); it++)
        {
            //遍历vector，找到集合I中状态经~所能到达的状态，并加入closureset中
            int count = NFAM.transfunc.size();
            for (int i = 0; i < count; i++)
            {

                //修改判定条件NFAM.transfunc[i].start要在closureset中！！！！！！！！！！
                if (closureset.count(NFAM.transfunc[i].start) == 1 && NFAM.transfunc[i].receive == '~')
                {
                    closureset.insert(NFAM.transfunc[i].end);
                }
            }
        }
        return closureset;
    }

    //求状态集合I经弧a或b或...到达的集合  只包含能到达的状态，不包含自身
    std::set<char> state_word_move(std::set<char> I, char receive, struct NFA NFAM)
    {
        std::set<char> wordmove_set;//最后的结果

        //对集合I每个状态遍历，找到其收到word所能到达的点
        for (std::set<char>::iterator it = I.begin(); it != I.end(); it++)
        {
            //遍历vector，找到集合I中状态经~所能到达的状态，并加入closureset中
            int count = NFAM.transfunc.size();
            for (int i = 0; i < count; i++)
            {
                if (NFAM.transfunc[i].start == (*it) && NFAM.transfunc[i].receive == receive)
                {
                    wordmove_set.insert(NFAM.transfunc[i].end);
                }
            }
        }
        return wordmove_set;
    }

    //状态集重命名
    std::map<std::set<char>, char> rename(std::vector<std::set<char>> I)
    {
        std::map<std::set<char>, char> rename_table;
        for (unsigned int i = 0; i < I.size(); i++)
        {
            if (i == 0)
            {
                rename_table[I[i]] = 'X';
            } else if (i == I.size() - 1)
            {
                rename_table[I[i]] = 'Y';
            } else
            {
                if (!I[i].empty())
                    rename_table[I[i]] = '0' + i - 1;
            }
        }

        return rename_table;
    }

    bool ASC(trans i, trans j)
    {
        return (i.start < j.start);
    }//升序 给std::sort用
    bool DESC(trans i, trans j)
    {
        return (i.start > j.start);
    }//降序 给std::sort用

    void resort_trans(struct DFA &DFAM)
    {
        //XX 00 11 22 YY
        //X 0 Y
        sort(DFAM.transfunc.begin(), DFAM.transfunc.end(), DESC);//这步结束时 X,Y,2,1,0,目的是把X,Y放最前面
        //循环找到第一个数字start状态
        unsigned int index = 0;
        for (index = 0; index < DFAM.transfunc.size(); index++)
        {
            if (DFAM.transfunc[index].start < 'X')
            {
                break;
            }
        }

        // XY 排序
        sort(DFAM.transfunc.begin(), DFAM.transfunc.begin() + index, ASC);//这步结束时 X,Y,0,1,2
        // 数字排序
        sort(DFAM.transfunc.begin() + index, DFAM.transfunc.end(), ASC);//这步结束时 X,Y,0,1,2
    }


    //NFA 转 DFA
    DFA NFAtoDFA(struct NFA NFAM)
    {

        //int wordnum;//字母数量，除了空字符
        //wordnum = NFAM.word.size() - 1;

        std::vector<std::set<char>> I;//初状态集
        std::queue<std::set<char>> new_state_set;//产生的新状态，保存在队列中，每个循环取出第一个新状态放在vector  I中
        std::vector<std::set<char>> old_state_set;//旧的状态集，new_state_set每次用完一个状态集，就放到old_state_set中，后面产生的状态集在old_state_set
        //中查询是否为新状态集   这里用vector存储而不用queue是因为队列不能遍历

        std::map<std::set<char>, char> rename_table;//重命名字典，每个集合对应一个新状态名

        struct DFA DFAM;//最后的转化结果

        //转换表的存储模型
        //vector              map
        //                    key
        //  I             a     b     c
        //{X,1,2}       {1,2} {1,2} {1,2}
        //{1,2}         {1,2} {1,2} {1,2}

        //求出初态
        I.push_back(closure({'X'}, NFAM));

        //遍历字母表（不包含~），建立map，map的key为word，value为vector，vector存放set
        // key         a   b   c...
        // value     {1,2}
        //           {3,4}
        //            ...
        //在此处给map的value插入第一组set，相当于构建了状态集表的第一行
        std::map<char, std::vector<std::set<char>>> word_move_map;
        for (std::set<char>::iterator it = NFAM.word.begin(); it != NFAM.word.end(); it++)
        {
            if (*it == '~')
            {
                continue;
            }
            std::set<char> tempset;
            tempset = state_word_move(I[0], *it, NFAM);
            tempset = closure(tempset, NFAM);
            word_move_map[*it].push_back(tempset);

            std::set<char> differ_tempset1;
            std::set<char> differ_tempset2;
            set_difference(tempset.begin(), tempset.end(), I[0].begin(), I[0].end(),
                           inserter(differ_tempset1, differ_tempset1.begin()));
            set_difference(I[0].begin(), I[0].end(),tempset.begin(), tempset.end(),
                           inserter(differ_tempset2, differ_tempset2.begin()));
            //集合非空且不和初态相同
            if (!tempset.empty() && (!differ_tempset1.empty()||!differ_tempset2.empty()))
            {
                new_state_set.push(tempset);//新状态集放在队列中
            }
        }

        //正式开始对状态集表进行处理
        while (new_state_set.size() != 0)
        {
            std::set<char> newpop_tempset;//临时集合，用来存放new队列中pop出来的集合
            newpop_tempset = new_state_set.front();
            new_state_set.pop();//pop没有返回值，所以先用front接受即将pop出的集合
            I.push_back(newpop_tempset);//用tempset作为初状态集I

            //作为初态集后就立即添加到old_state_set中表示状态已经用过
            old_state_set.push_back(newpop_tempset);
            //在循环中求出通过每个字母产生的集合
            for (std::set<char>::iterator it = NFAM.word.begin(); it != NFAM.word.end(); it++)
            {
                if (*it == '~')
                {
                    continue;
                }
                std::set<char> tempset;
                tempset = state_word_move(newpop_tempset, *it, NFAM);
                tempset = closure(tempset, NFAM);
                //放到map中
                word_move_map[*it].push_back(tempset);

                //在old_state_set中对比，看tempset是不是新状态集，是则放进new_state_set队列中
                int is_new_state = 1;//是否是新集合
                for (unsigned int i = 0; i < old_state_set.size(); i++)
                {
                    std::set<char> differ_tempset;
                    set_difference(tempset.begin(), tempset.end(), old_state_set[i].begin(), old_state_set[i].end(),
                                   inserter(differ_tempset, differ_tempset.begin()));
                    //后一个条件防止是子集的情况发生
                    if (differ_tempset.size() == 0 && tempset.size() == old_state_set[i].size())
                    {//equal的判断集合相等的写法，但OJ编译不通过，只能用集合差  equal(tempset.begin(), tempset.end(), old_state_set[i].begin(), old_state_set[i].end())
                        //如果有相等的集合，则不是新集合
                        is_new_state = 0;
                        break;
                    }
                }
                //是新集合且不为空集合，new_state_set队列中
                if (is_new_state == 1 && !tempset.empty())
                {
                    new_state_set.push(tempset);
                }

            }

        }//while

        //重命名表生成
        rename_table = rename(I);

        //转化为DFA，一边查表一边转化为DFA
        DFAM.word = NFAM.word;//复制字母表
        DFAM.word.erase('~');//去掉空字符
        for (unsigned int i = 0; i < I.size(); i++)
        {
            //遍历vector I，I和map是每行对应的
            //DFAM插入状态

            DFAM.state.insert(rename_table[I[i]]);

            //DFAM每个状态对应的转换关系进行添加 遍历word相当于在横向遍历map
            for (std::set<char>::iterator it = DFAM.word.begin(); it != DFAM.word.end(); it++)
            {
                struct trans DFAMtemptrans;
                //是空集合则跳过转换关系添加
                if (word_move_map[*it][i].empty())
                {
                    continue;
                }
                DFAMtemptrans.start = rename_table[I[i]];
                DFAMtemptrans.receive = *it;;
                DFAMtemptrans.end = rename_table[word_move_map[*it][i]];
                DFAM.transfunc.push_back(DFAMtemptrans);
            }
        }

        //对DFA的transfunc进行重新排序，达到用例格式 X,Y,0,1,2.....
        resort_trans(DFAM);

        return DFAM;

    }


    void DFAoutput(struct DFA DFAM)
    {
        unsigned int j = 0;
        int countline = 0;//2020.12.29
        char nowstate;
        for (unsigned int i = 0; i < DFAM.transfunc.size(); i++)
        {
            if (j == 0)
            {
                nowstate = DFAM.transfunc[i].start;

                if (nowstate != 'Y' && countline == 1)
                {//2020.12.29
                    printf("Y \n");       //2020.12.29
                }//2020.12.29

                printf("%c ", DFAM.transfunc[i].start);
                j++;
            }
            if (j != 0)
            {
                if ((i + 1) < DFAM.transfunc.size())
                {
                    if (DFAM.transfunc[i + 1].start == nowstate)
                    {
                        printf("%c-%c->%c ", DFAM.transfunc[i].start, DFAM.transfunc[i].receive, DFAM.transfunc[i].end);
                    } else
                    {
                        printf("%c-%c->%c\n", DFAM.transfunc[i].start, DFAM.transfunc[i].receive,
                               DFAM.transfunc[i].end);
                        j = 0;
                        countline++;//2020.12.29
                    }
                } else
                {
                    printf("%c-%c->%c", DFAM.transfunc[i].start, DFAM.transfunc[i].receive, DFAM.transfunc[i].end);
                }
            }

        }

    }

    void DFAoutput2File(struct DFA DFAM)
    {
        FILE *fout = fopen("tmp_nfa2dfa.txt", "w");
        unsigned int j = 0, markA = 0;
        char nowstate;
        for (unsigned int i = 0; i < DFAM.transfunc.size(); i++)
        {
            if (j == 0)
            {
                nowstate = DFAM.transfunc[i].start;
                fprintf(fout, "%c ", DFAM.transfunc[i].start);
                j++;
            }
            if (j != 0)
            {
                if ((i + 1) < DFAM.transfunc.size())
                {
                    if (DFAM.transfunc[i + 1].start == nowstate)
                    {
                        fprintf(fout, "%c-%c->%c ", DFAM.transfunc[i].start, DFAM.transfunc[i].receive,
                                DFAM.transfunc[i].end);
                    } else
                    {
                        fprintf(fout, "%c-%c->%c\n", DFAM.transfunc[i].start, DFAM.transfunc[i].receive,
                                DFAM.transfunc[i].end);
                        j = 0;
                        if (markA == 0)
                        {
                            fprintf(fout, "Y \n");
                            markA++;
                        }
                    }
                } else
                {
                    fprintf(fout, "%c-%c->%c", DFAM.transfunc[i].start, DFAM.transfunc[i].receive,
                            DFAM.transfunc[i].end);
                }
            }
        }
        fprintf(fout, "\n\n");
        fclose(fout);
    }

    void nfa2dfa()
    {
        struct nfa2dfa::NFA nfam;
        struct nfa2dfa::DFA dfam;
        input(nfam);
        dfam = NFAtoDFA(nfam);
        DFAoutput(dfam);
        DFAoutput2File(dfam);
    }
}

namespace dfaSimplify
{
    typedef struct PathList
    { //存一条通路
        char cDFA_State[20];
        int underse_sum = 0;
    } PathList;
    PathList pList[10];
    typedef struct DFA_State
    { //存边
        char Startname, Endname;
        char Condition;
        int yesEdge;

        bool operator==(const DFA_State &s)
        {
            return ((Startname == s.Startname) && (Condition == s.Condition)
                    && (Endname == s.Endname) && (yesEdge == s.yesEdge));
        }
    } DFA_State;

    class DFA
    {
    public:
        std::vector<DFA_State> dfaStateList; //存边
        std::vector<std::vector<char> > splitStates; //存分割子集
        void input();

        void elimDeadState();

        int DFSTraverse(char S_tart, PathList pList[]); //开始坐标、通路数组
        void DFS(char S_tart, bool bVisited[], int &nIndex, PathList pList[], int &sum);

        int find_Edge(char S_tart, char S_end);

        int find_Char(char State);

        void simple();

        int findset(char Endname);

        void split(int i, std::map<int, std::vector<char> > &res);


        bool is_element_in_vector(char Startname, std::vector<char> dfaState);

        void merge();

        void output();
    };

    void dfaSimplify::DFA::input()
    {   //用于输入DFA信息
        std::streambuf *cinBuf = std::cin.rdbuf();
        std::ifstream inf("tmp_nfa2dfa.txt");
        std::streambuf *fileBuf = inf.rdbuf();
        std::cin.rdbuf(fileBuf);

        DFA_State dfaState[20];
        for (int i = 0; i < 20; i++) dfaState[i].yesEdge = 1;
        int j = 0;
        std::string str;
        std::vector<char> EndSt, noEndSt;
        getline(std::cin, str, '\n');
        while (str.length() != 0)
        {
            if (str.length() == 1)
            {
                if (str[0] == 'Y') EndSt.push_back(str[0]);
                else noEndSt.push_back(str[0]);
                getline(std::cin, str, '\n'); //输入string串
            } else
            {
                dfaState[j].Startname = str[0];//边起始
                for (int i = 1; str[i] != '\0'; i++)
                {
                    if (str[i - 1] == '-' && str[i + 1] == '-') dfaState[j].Condition = str[i];//条件
                    else if (str[i] == '>')
                    {
                        dfaState[j].Endname = str[++i]; //边终点
                        //测试代码
                        //cout<<dfaState[j].Startname<<' '<<dfaState[j].Condition<<' '<<dfaState[j].Endname<<endl;
                        dfaStateList.push_back(dfaState[j++]);
                        dfaState[j].Startname = str[0];
                    }
                }
                if (str[0] == 'Y') EndSt.push_back(str[0]);
                else noEndSt.push_back(str[0]);
                getline(std::cin, str, '\n'); //输入string串
            }
        }
        splitStates.push_back(noEndSt), splitStates.push_back(EndSt);
        // for(int i=0;i<dfaStateList.size();i++)
        //{
        // cout<<dfaStateList[i].Startname<<' '<<dfaStateList[i].Condition
        //<<' '<<dfaStateList[i].Endname;
        // cout<<endl;
        // }

        inf.close();
        std::cin.rdbuf(cinBuf);
    }

    int dfaSimplify::DFA::findset(char Endname)
    { //还未划分时 非终结符、终结符各一行。用于获得某状态属于哪个集合
        for (std::size_t i = 0; i < splitStates.size(); i++)
            for (std::size_t j = 0; j < splitStates.size(); j++)
                if (Endname == splitStates[i][j])
                    return i;
    }

    int dfaSimplify::DFA::find_Edge(char S_tart, char S_end) //用于找出以S_tart为起始,以S_end为终态的边
    {
        for (std::size_t i = 0; i < dfaStateList.size(); i++)
            if (dfaStateList[i].Startname == S_tart && dfaStateList[i].Endname == S_end)
                return i;
        return -1;
    }

    int DFA::find_Char(char State) //用于找出某状态是否存在于状态集合中
    {
        for (std::size_t i = 0; i < splitStates.size(); i++)
            for (std::size_t j = 0; j < splitStates[i].size(); j++)
                if (State == splitStates[i][j]) return i;
        return -1;
    }

    //DFS遍历得到从起始点到终态的所有通路
    void DFA::DFS(char S_tart, bool bVisited[], int &nIndex, PathList pList[], int &sum)
    {
        bVisited[int(S_tart)] = true; //改为已访问
        pList[sum].cDFA_State[nIndex++] = S_tart;//访问S_tart状态

        int num = 0;
        for (std::size_t i = 0; i < splitStates.size(); i++)
            for (std::size_t j = 0; j < splitStates[i].size(); j++)
            {
                if (bVisited[int(splitStates[i][j])])
                    num++; //边上总共有几个点
            }

        if (pList[sum].cDFA_State[nIndex - 1] == 'Y') //找到一条通路
        {
            if (sum != 0) pList[sum].cDFA_State[0] = 'X'; //只有第一条通路的第一个点被赋值为了初态.所以这里有个判断
            pList[sum].underse_sum = num;
            sum++;
            //保存一条路径
        } else
        {
            for (std::size_t i = 0; i < splitStates.size(); i++)
                for (std::size_t j = 0; j < splitStates[i].size(); j++)
                {
                    int number = find_Edge(S_tart, splitStates[i][j]);
                    if (number >= 0 && !bVisited[int(splitStates[i][j])]) //有边并且这个点没访问过(防止形成环)
                    {
                        DFS(splitStates[i][j], bVisited, nIndex, pList, sum);
                        bVisited[int(splitStates[i][j])] = false;
                        nIndex--;
                    }
                }
        }
    }

    //用于调用DFS得到所有通路,返回通路数
    int DFA::DFSTraverse(char S_tart, PathList pList[])
    {
        bool bVisted[100] = {false};
        int nIndex = 0, sum = 0;
        DFS(S_tart, bVisted, nIndex, pList, sum);
        return sum;
        //从S_tart开始状态,bVisited记录是否访问过,nIndex记录状态数,pList为通路链表,sum为总通路数
    }

    //思路：找从起点开始找到终结点的通路。
    //       遍历各个通路,如果某点不存在任何一个通路上,则证明该点为死状态点
    //用于消除死状态、死状态对应的边
    void DFA::elimDeadState()
    {
        //从开始顶点X找到各个通路
        char S_tart = 'X';
        int s_Pathway = DFSTraverse(S_tart, pList);

        //遍历各个通路,遍历过程中更新状态集合以及边集合
        //更新状态
        std::vector<char> noEndSt, EndSt;
        EndSt.push_back('Y');
        for (int i = 0; i < s_Pathway; i++)
            for (int under = 0; under < pList[i].underse_sum; under++)
            {
                char Ele_st = pList[i].cDFA_State[under];
                if (find(noEndSt.begin(), noEndSt.end(), Ele_st) == noEndSt.end() && Ele_st != 'Y')
                    noEndSt.push_back(Ele_st);
            }
        splitStates.clear();
        splitStates.push_back(noEndSt);
        splitStates.push_back(EndSt);
        /*for(int i=0;i<splitStates.size();i++)
        {

            for(int j=0;j<splitStates[i].size();j++)
                cout<<splitStates[i][j];
            cout<<endl;
        }*/


        //更新边集合
        std::vector<DFA_State> dfaStateList2;
        for (std::size_t i = 0; i < dfaStateList.size(); i++)
        {
            char S_tart = dfaStateList[i].Startname;
            char S_end = dfaStateList[i].Endname;
            if (find_Char(S_tart) >= 0 && find_Char(S_end) >= 0)
                dfaStateList2.push_back(dfaStateList[i]);
        }
        dfaStateList.clear();
        for (std::size_t i = 0; i < dfaStateList2.size(); i++)
        {
            dfaStateList.push_back(dfaStateList2[i]);
        }

        /*for(int i=0;i<dfaStateList.size();i++)
        {
            cout<<dfaStateList[i].Startname<<' '<<dfaStateList[i].Condition<<' '<<dfaStateList[i].Endname;
            cout<<endl;
        }*/



        /*for(int i=0;i<s_Pathway;i++) //输出各通路信息,用于测试
        {
            for(int under=0;under<pList[i].underse_sum;under++)
              cout<<pList[i].cDFA_State[under];
            cout<<endl;
        }*/
    }

    //用于对splitStates[i]集合,以res分割集合进行分割
    void DFA::split(int i, std::map<int, std::vector<char> > &res)
    {
        std::map<int, std::vector<char> >::iterator iter;
        iter = res.begin();

        splitStates[i].clear();
        splitStates[i].assign((iter->second).begin(), (iter->second).end());
        iter++;

        while (iter != res.end())
        {
            splitStates.push_back(iter->second);
            iter++;
        }

        /*for(int i=0;i<splitStates.size();i++)
           {
               for(int j=0;j<splitStates[i].size();j++)
                 cout<<splitStates[i][j]<<" ";
               cout<<endl;
           }
        cout<<endl;*/ //测试代码
    }

    //使用分割法进行状态化简
    void DFA::simple()
    {
        char End[] = {'a', 'b', 'c','d','e','f','g','A','B','C','D','E','F', '~', '0', '1', '\0'};
        std::map<int, std::vector<char> > res;
        bool flag = false;
        std::size_t i = 0;
        for (i = 0; i < splitStates.size(); i++)
        { //对各状态子集进行不断分割
            if (splitStates[i].size() > 1)
            {
                flag = false;
                for (int endChar = 0; endChar < sizeof(End)/sizeof(char); endChar++)
                { //对于每个输入
                    for (std::size_t j = 0; j < splitStates[i].size(); j++)
                    { //每个子集有splitStates[i].size()个元素
                        bool haveEdge = false;
                        for (std::size_t k = 0; k < dfaStateList.size(); k++)
                        {
                            char Edge_Start = dfaStateList[k].Startname;
                            if (Edge_Start == splitStates[i][j] && dfaStateList[k].Condition == End[endChar])
                            {
                                int number = findset(dfaStateList[k].Endname); //找到该弧转换到的状态所属的划分集合号
                                if (res.count(number) == 0)
                                    res.insert(
                                            std::pair<int, std::vector<char> >(number,
                                                                               std::vector<char>(1, Edge_Start)));
                                else res[number].push_back(Edge_Start);
                                haveEdge = true;
                                break;
                            }
                        }
                        if (!haveEdge)
                        {
                            if (res.count(-1) == 0)
                                res.insert(std::pair<int, std::vector<char> >(noEdge,
                                                                              std::vector<char>(1, splitStates[i][j])));
                            else
                                res[-1].push_back(splitStates[i][j]);
                        }

                    }
                    if (res.size() > 1)
                    {
                        split(i, res); //第i个集合,通过res字典划分
                        flag = true;
                        res.clear();
                        break;
                    }
                    res.clear();
                }
                if (flag) i = -1;//说明发生了集合分割,需要从头开始遍历。
            }
        }
    }

    //判断某name在vector中是否存在
    bool DFA::is_element_in_vector(char Startname, std::vector<char> dfaState)
    {
        for (std::size_t i = 0; i < dfaState.size(); i++)
            if (Startname == dfaState[i]) return true;
        return false;
    }

    //用于状态合并
    void DFA::merge()
    {
        std::map<std::vector<char>, char> Old_result;
        std::vector<char> sameState;
        for (std::size_t i = 0; i < splitStates.size(); i++)
        {
            sameState.clear();
            for (std::size_t j = 0; j < splitStates[i].size(); j++) sameState.push_back(splitStates[i][j]);
            Old_result.insert(std::pair<std::vector<char>, char>(sameState, splitStates[i][0]));
        } //map容器复制 各子集为key,标志状态为value

        for (std::size_t k = 0; k < dfaStateList.size(); k++)  //对于每条边
        {
            char Start = dfaStateList[k].Startname;
            char End = dfaStateList[k].Endname; //得到起点和终点
            for (std::size_t i = 0; i < splitStates.size(); i++)
            {
                if (is_element_in_vector(Start, splitStates[i]) && is_element_in_vector(End, splitStates[i]) &&
                    Start != End)
                {
                    dfaStateList[k].yesEdge = -1; //该边删 置标志位为-1.
                    break;
                }
            }
            for (std::size_t i = 0; i < splitStates.size(); i++)
            {
                //修改边的始末点为map中的value
                if (is_element_in_vector(Start, splitStates[i])) dfaStateList[k].Startname = Old_result[splitStates[i]];
                if (is_element_in_vector(End, splitStates[i])) dfaStateList[k].Endname = Old_result[splitStates[i]];
            }
        }

        //去重
        std::vector<DFA_State> dfaStatesNew;
        for (std::size_t i = 0; i < dfaStateList.size(); i++)
        {
            if (find(dfaStatesNew.begin(), dfaStatesNew.end(), dfaStateList[i]) == dfaStatesNew.end())
            {
                dfaStatesNew.push_back(dfaStateList[i]);
            }
        }
        //输出 先输出ab# XY02#
        std::vector<char> character;
        for (std::size_t k = 0; k < dfaStatesNew.size(); k++)
        {
            //将输入符push到vector中
            if (std::find(character.begin(), character.end(), dfaStatesNew[k].Condition) == character.end())
                character.push_back(dfaStatesNew[k].Condition);
        }
        //输出a b#
        for (std::size_t k = 0; k < character.size() - 1; k++)
            std::cout << character[k] << ' ';
        std::cout << character[character.size() - 1] << '#' << std::endl;

        //输出X Y 0 2#
        for (std::size_t i = 0; i < splitStates.size() - 1; i++)
            std::cout << splitStates[i][0] << ' ';
        std::cout << splitStates[splitStates.size() - 1][0] << '#' << std::endl;

        //输出DFA
        for (std::size_t i = 0; i < splitStates.size(); i++) //对于每个splitStates[i][0]
        {
            if(splitStates[i][0]=='X'||splitStates[i][0]=='Y'||splitStates[i][0]<='9'&&splitStates[i][0]>='0'){

            }else{
                continue;
            }
            std::cout << splitStates[i][0] << ' ';
            for (std::size_t k = 0; k < dfaStatesNew.size(); k++)
            {
                //如果不为-1并且起始点为splitStates[i][0]
                if (dfaStatesNew[k].yesEdge != -1 && dfaStatesNew[k].Startname == splitStates[i][0])
                {
                    std::cout << dfaStatesNew[k].Startname << '-' << dfaStatesNew[k].Condition << '-' << '>'
                              << dfaStatesNew[k].Endname;
                    std::cout << ' ';
                }
            }
            std::cout << std::endl;
        }

        //输出到文件
        std::streambuf *coutBuf = std::cout.rdbuf();
        std::ofstream of("tmp_dfa_simpilify.txt");
        std::streambuf *fileBuf = of.rdbuf();
        std::cout.rdbuf(fileBuf);

        //输出a b#
        for (std::size_t k = 0; k < character.size() - 1; k++)
            std::cout << character[k] << ' ';
        std::cout << character[character.size() - 1] << '#' << std::endl;

        //输出X Y 0 2#
        for (std::size_t i = 0; i < splitStates.size() - 1; i++)
            std::cout << splitStates[i][0] << ' ';
        std::cout << splitStates[splitStates.size() - 1][0] << '#' << std::endl;

        //输出DFA
        for (std::size_t i = 0; i < splitStates.size(); i++) //对于每个splitStates[i][0]
        {
            if(splitStates[i][0]=='X'||splitStates[i][0]=='Y'||splitStates[i][0]<='9'&&splitStates[i][0]>='0'){

            }else{
                continue;
            }
            std::cout << splitStates[i][0] << ' ';
            for (std::size_t k = 0; k < dfaStatesNew.size(); k++)
            {
                //如果不为-1并且起始点为splitStates[i][0]
                if (dfaStatesNew[k].yesEdge != -1 && dfaStatesNew[k].Startname == splitStates[i][0])
                {
                    std::cout << dfaStatesNew[k].Startname << '-' << dfaStatesNew[k].Condition << '-' << '>'
                              << dfaStatesNew[k].Endname;
                    std::cout << ' ';
                }
            }
            std::cout << std::endl;
        }
        of.flush();
        of.close();
        std::cout.rdbuf(coutBuf);
    }

    void dfaSimplify()
    {
        DFA dfa;
        dfa.input();
        dfa.elimDeadState();
        dfa.simple();

        dfa.merge();
    }
}

namespace dfaIdentity
{

    class DFA
    {
    public:
        std::set<char> word;   //单词集合
        std::set<char> status; //状态集合
        std::map<std::string, std::string> dfa;   //DFA关系集合
        std::string Str = "";

        // 利用文件生成dfa关系
        void creat_dfa_txt()
        {
            std::ifstream infile;
            infile.open("tmp_dfa_simpilify.txt");
            std::string Str;
            int z = 0;
            while (getline(infile, Str))
            {
                if (z < 1)
                {
                    for (int i = 0; i < Str.size(); i++)
                    {
                        if (Str[i] != ' ' && Str[i] != '#')
                        {
                            word.insert(Str[i]);
                        }
                    }
                } else
                {
                    std::string out = "";
                    std::istringstream str(Str);
                    str >> out;
                    std::string pro = out;
                    while (str >> out)
                    {
                        if (out.length() == 1)
                            break;
                        else
                        {
                            std::string f_c = "";
                            f_c = f_c + pro;
                            int i = 1 + out.find('-');
                            // 生成关系字符串
                            while (out[i] != '-')
                            {
                                f_c = f_c + out[i];
                                i++;
                            }
                            // 关系变成字典
                            dfa[f_c] = out.substr(i + 2, out.size() - i - 2);
                        }
                    }
                }
                z++;
            }
        }

        // 单词识别函数
        void get_string()
        {
            char c;
            int z = 0;
            c = getchar();
            while ((c = getchar()) != EOF)
            {
                if (c != '#')
                {
                    Str = Str + c;
                } else
                {
                    Str = Str + c;
                    c = getchar();
                    if (c == '\n')
                    {
                        c = getchar();
                        if (c == '\n') z++;
                        else Str = Str + c;
                    }
                }
                if (z == 1) break;
            }
        }

        void find_if()
        {
            int n = Str.length();
            std::string c = "X";
            for (int i = 0; i < n; i++)
            {
                if (Str[i] != '#')
                {
                    std::string s;
                    s = s + c;
                    s = s + Str[i];
                    c = dfa[s];
                    // 字符是否存在
                    if (c.length() > 0)
                    {
                        std::cout << Str[i] << std::endl;
                    } else
                    {
                        std::cout << "error" << std::endl;
                        while (Str[i] != '#')
                        {
                            i++;
                        }
                        c = "X";
                    }
                }
                    // 单个字符串结束判断
                else
                {
                    if (c[0] == 'Y')
                    {
                        std::cout << "pass" << std::endl;
                    } else
                    {
                        std::cout << "error" << std::endl;
                    }
                    c = 'X';
                }
            }
        }
    };

    void dfaIdentity()
    {
        DFA dfa;
        // 录入信息方法
        dfa.creat_dfa_txt();
        dfa.get_string();
        // 单词识别方法
        dfa.find_if();
    }
}


int main()
{
    return 0;
}
