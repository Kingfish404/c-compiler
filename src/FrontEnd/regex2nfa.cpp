// -*- C++ -*-
// @file regex2nfa.cpp
// @describe this cpp file is use to translate the regular expression to NFA
// @描述 此文件用于将正则表达式转换为NFA
// @author Jin Yu
// @date 2020-11-18
// @myblog https://blog.achacker.com
// @直接运行的预期输出如下：
// This is input: (a|b)*abb
// This is inserted: (a|b)*+a+b+b
// This is inserted: (a|b)*+a+b+b
// This is postfix: ab|*a+b+b+
// -1 -1-~->0 
// 0 0-~->1 0-a->0 0-b->0 
// 1 1-a->2 
// 2 2-b->3 
// 3 3-b->4 

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>

#define DIV_CHAR '+' // 这是用来分隔的字符，表示连接

bool CHECK_ON = true; // 是否开启检查内容的输入输出

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
    std::string input;       // 原始的输入字符串
    std::string strBeInsert; // 插入分隔符后的字符串
    std::string strPostfix;
    std::string alphaBet;            // 字母表
    std::map<char, int> op_priority; // 运算符优先级
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
                std::cout << state->ID << "-" << pairTemp.first << "->" << pairTemp.second->ID << " ";
            }
            std::cout << std::endl;
        }
    }

    void output()
    {
        // 为了防止抄袭，我把符合课程OJ的结果删除了
        std::cout << "为了防止抄袭，我把符合课程OJ的结果删除了" << std::endl;
    }

public:
    // 向正则表达式中插入分隔符号
    void insertExplicit()
    {
        strBeInsert = input;
        bool shouldInsert = false;
        int count = 0; // 用于标记当前扫描到的字符
        for (unsigned int i = 0; i < input.length() - 1; i++, count++)
        {
            // 对于如ab,a(c),a*(c),(a)b,a*b,五种状态
            // 替换成a_b,a_(c),a*_(c),(a)_b,a*_b
            if (isCharInStr(alphaBet, strBeInsert[count]) &&
                (isCharInStr(alphaBet, strBeInsert[count + 1]) || strBeInsert[count + 1] == '('))
            {
                // 对于两个字母相邻，插入分隔
                shouldInsert = true;
            }
            else if (strBeInsert[count] == '*' &&
                     strBeInsert[count + 1] == '(')
            {
                // 对于字母或者'*'与'('相邻，插入分隔
                shouldInsert = true;
            }
            else if (strBeInsert[count] == ')' && isCharInStr(alphaBet, strBeInsert[count + 1]))
            {
                // 对于')'和字母相邻，插入分隔
                shouldInsert = true;
            }
            else if (strBeInsert[count] == '*' && isCharInStr(alphaBet, strBeInsert[count + 1]))
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
            }
            else
            {
                if (op_stack.empty() || ch == '(')
                {
                    op_stack.push_back(ch);
                }
                else if (ch == ')')
                {
                    while (op_stack.back() != '(')
                    {
                        tempChar = op_stack.back();
                        outputPostfix.push_back(tempChar);
                        op_stack.pop_back();
                    }
                    op_stack.pop_back();
                }
                else
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
        }
        else
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

int main()
{
    Regex2Nfa regex2nfa;

    // get regex expression,eg: (a|b)*c,abed
    regex2nfa.setInput("(a|b)*abb");
    //    regex2nfa.getInput(); // get input str from keyboard
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
        // [Check]check the string after being insert
        regex2nfa.printInsertStrByChar();
        // [Check]check postfix string
        regex2nfa.printPostfixStrByChar();
    }

    // [Check]Output the final NFA
    regex2nfa.outputOrigin();

    return 0;
}
