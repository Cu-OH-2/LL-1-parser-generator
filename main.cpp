/* 编译原理大作业4-LL(1)分析法 第一小组 林鑫宇 伊然 毛凌骏 梁馨 */
#include<iostream>
#include<vector>
#include<map>
#include<set>
#include<string>
#include<iomanip>
#include<stack>
using namespace std;

//#define debug

inline bool isVn(char ch) { return ch >= 'A' && ch <= 'Z'; } //判断是否非终结符

class SyntacticParser; //提前声明
/*-----------------------------------------------------------------------*/
struct Production
{
    char lef; //产生式左侧非终结符
    string rig; //产生式右部

    //重载输入
    friend istream& operator >> (istream& in, Production& p)
    {
        string read;
        in >> read;
        p.lef = read.front();
        p.rig = read.substr(3);
        return in;
    }

    //重载输出
    friend ostream& operator << (ostream& out, const Production& p)
    {
        out << p.lef << "→" << p.rig; 
        return out;
    }
};
/*-----------------------------------------------------------------------*/
class Grammar
{
    friend class SyntacticParser;

protected:
    vector<Production> prods; //产生式
    map<char, vector<int>> to; //每个非终结符对应的产生式编号

    void showTips() const; //显示提示信息
    void input(); //输入
public:
    Grammar(); //根据输入构造文法
    void print(); //输出
};

void Grammar::showTips() const
{
    cout << setiosflags(ios::left);
    cout << "+------------------------------------------------+" << endl;
    cout << "|" << setw(48) << "文法输入注意事项：" << '|' << endl;
    cout << "|" << setw(48) << "1、所有终结符、非终结符都用单个英文字母表示" << '|' << endl;
    cout << "|" << setw(48) << "2、终结符用小写字母表示，非终结符用大写字母表示" << '|' << endl;
    cout << "|" << setw(48) << R"(3、空字epsilon用"e"表示)" << '|' << endl;
    cout << "|" << setw(48) << R"(4、箭头用"→"或"->"表示")" << '|' << endl;
    cout << "|" << setw(48) << "5、一条产生式的所有字符间不含空格" << '|' << endl;
    cout << "|" << setw(48) << "6、输入的文法应满足LL(1)文法要求" << '|' << endl;
    cout << "|" << setw(48) << "7、输入的第一条产生式应为文法起始符的产生式" << '|' << endl;
    cout << "+------------------------------------------------+" << endl;
    cout << resetiosflags(ios::left);
    return;
}

void Grammar::input()
{
    int cntp = 0;
    Production p;
    cout << "\n请输入文法产生式条数：" << endl;
    cin >> cntp;
    cout << "\n请输入产生式：" << endl;
    while (cntp--)
    {
        //输入产生式
        cin >> p;

        //添加产生式
        to[p.lef].push_back(prods.size());
        prods.push_back(p);
    }
    return;
}

Grammar::Grammar()
{
    showTips();
    input();
#ifdef debug
    print();
#endif;
}

void Grammar::print()
{
    cout << "\n文法产生式为：" << endl;
    for (auto& p : prods) cout << p << endl;
    return;
}
/*-----------------------------------------------------------------------*/
class SyntacticParser
{
protected:
    set<char> Vn, Vt; //非终结符集和终结符集
    map<pair<char,char>, Production> parTable; //LL(1)分析表
    map<char, set<char>> first, follow; //FIRST集合与FOLLOW集合
    map<char, bool> eps; //非终结符是否可推导为空字
    char start; //文法起始符

    void getV(const Grammar& g); //求Vt集和Vn集
    void getFirstOf(const Grammar& g, const char c); //求FIRST(c)
    void getFirst(const Grammar& g); //求所有非终结符的FIRST集合
    void getFollow(const Grammar& g); //求所有非终结符的FOLLOW集合
    void getParsingTable(const Grammar& g); //求LL(1)分析表

public:
    SyntacticParser(const Grammar& g); //根据文法g构造语法分析器
    void showFirst() const; //输出所有非终结符的FIRST集合
    void showFollow() const; //输出所有非终结符的FOLLOW集合
    void printParsingTable(); //打印LL(1)分析表
    int analyze(string sentence); //分析语句
    void visibleAnalyze(string sentence); //分析语句并显示分析过程
};

void SyntacticParser::getV(const Grammar& g)
{
    for (auto& e : g.to) Vn.insert(e.first);
    for (auto& e : g.prods) for (auto c : e.rig) if (!isVn(c) && c != 'e') Vt.insert(c);
    Vt.insert('#');
    return;
}

void SyntacticParser::getFirstOf(const Grammar& g, const char c)
{
    for (auto pid : g.to.find(c)->second) //遍历c的产生式id
    {
        const string& rig = g.prods[pid].rig; //产生式右侧字符串
        for (auto v : rig) //遍历产生式右侧所有字符
        {
            if (isVn(v)) //v是非终结符
            {
                if (first[v].empty()) //FIRST(v)还没求
                {
                    getFirstOf(g, v); //递归求得FIRST(v)（不含左递归则不会死循环）
                }
                for (auto e : first[v]) //将FIRST(v)-{e}加入FIRST(c)
                {
                    if (e != 'e') first[c].insert(e); //除空字外都加入
                }
            }
            else //v是终结符
            {
                first[c].insert(v); //将v加入FIRST(c)
                if (v == 'e') eps[c] = 1; //v是空字，标记c
            }
            if (!eps[v]) break; //FIRST(v)不含空字，退出循环
        }
    }
    return;
}

void SyntacticParser::getFirst(const Grammar& g)
{
    //遍历非终结符，若还未求FIRST集合则求FIRST集合
    for (auto v : Vn) if (first[v].empty()) getFirstOf(g, v); 
    return;
}

void SyntacticParser::getFollow(const Grammar& g)
{   
    //文法起始符的FOLLOW元素中加入结束符"#"
    follow[start].insert('#'); 

    while (1)
    {
        //计算一轮操作前FOLLOW集合总大小
        int beforeSize = 0;
        for (auto& s : follow) beforeSize += s.second.size();

        //开始一轮操作
        for (auto& p : g.prods) //遍历所有产生式
        {
            for (int i = 0; i < p.rig.size(); ++i) //遍历产生式右部所有符号
            {
                const char c = p.rig[i]; //产生式右部某个字符
                if (isVn(c)) //c是非终结符
                {
                    bool endFlag = 1; //p.lef是否能推导出...c
                    for (int j = i + 1; j < p.rig.size(); ++j) //遍历c右部字符
                    {
                        const char v = p.rig[j]; //c右部某个字符
                        if (isVn(v)) //v是非终结符
                        {
                            for (auto e : first[v]) //将FIRST(v)-{e}加入FOLLOW(c)
                            {
                                if (e != 'e') follow[c].insert(e); //除空字外都加入
                            }
                            if (!eps[v]) //v不可推导为空字，退出
                            {
                                endFlag = 0;
                                break;
                            }
                        }
                        else //v是终结符，加入FOLLOW(c)并退出
                        {
                            follow[c].insert(v);
                            endFlag = 0;
                            break;
                        }
                    }
                    if (endFlag) //p.lef可以推导出...c
                    {
                        for (auto e : follow[p.lef]) //将FOLLOW(p.lef)加入FOLLOW(c)
                        {
                            follow[c].insert(e);
                        }
                    }
                }
            }
        }

        //计算一轮操作后FOLLOW集合总大小
        int afterSize = 0;
        for (auto& s : follow) afterSize += s.second.size();

        //FOLLOW集合不再增大，循环结束
        if (beforeSize == afterSize) break;
    }
    return;
}

void SyntacticParser::getParsingTable(const Grammar& g)
{
    for (auto vn : Vn) //遍历非终结符
    {
        for (auto vt : Vt) //遍历终结符
        {
            if (first[vn].count(vt)) //若vt属于FIRST(vn)，找产生式
            {
                for (auto pid : g.to.find(vn)->second) //遍历vn产生式id
                {
                    auto& p = g.prods[pid]; //vn的某个产生式
                    for (auto c : p.rig) //逐个遍历字符，判断产生式的FIRST是否包含vt
                    {
                        if (isVn(c)) //c是非终结符
                        {
                            if (first[c].count(vt)) parTable[{vn, vt}] = p; //若FIRST(c)包含vt则p为所求
                            if (!eps[c]) break; //c不能推导为空字，退出
                        }
                        else //c是终结符
                        {
                            if (c == vt) parTable[{vn, vt}] = p; //若c为vt则p为所求
                            break;
                        }
                    }
                    if (parTable.count({ vn, vt })) break; //所求产生式已经找到，退出循环
                }
            }
            else if (follow[vn].count(vt) && eps[vn]) //vt属于FOLLOW(vn)且vn可推导为空字
            {
                for (auto pid : g.to.find(vn)->second) //遍历vn产生式id
                {
                    auto& p = g.prods[pid]; //vn的某个产生式
                    if (p.rig.front() == 'e') //p为vn→e
                    {
                        parTable[{vn, vt}] = p; //将p作为vn遇到vt时使用的产生式
                        break;
                    }
                }
            }
        }
    }
    return;
}

SyntacticParser::SyntacticParser(const Grammar& g) :start(g.prods.front().lef)
{
    getV(g);
    getFirst(g);
    getFollow(g);
    getParsingTable(g);
    showFirst();
    showFollow();
    printParsingTable();
}

void SyntacticParser::showFirst() const
{
    cout << "\nFIRST集合：" << endl;
    auto printLine = []() {cout << '+' << setw(30) << setfill('-') << "" << endl; };
    printLine();
    for (auto e : first)
    {
        cout << "|FIRST(" << e.first << ") = { ";
        bool com = 0;
        for (auto ee : e.second) cout << (com ? ", " : "") << ee, com = 1;
        cout << " }" << endl;
    }
    printLine();
    return;
}

void SyntacticParser::showFollow() const
{
    cout << "\nFOLLOW集合：" << endl; 
    auto printLine = []() {cout << '+' << setw(30) << setfill('-') << "" << endl; };
    printLine();
    for (auto e : follow)
    {
        cout << "|FOLLOW(" << e.first << ") = { ";
        bool com = 0;
        for (auto ee : e.second) cout << (com ? ", " : "") << ee, com = 1;
        cout << " }" << endl;
    }
    printLine();
    return;
}

void SyntacticParser::printParsingTable()
{
    cout << "\nLL(1)分析表：" << endl;
    auto printLine = [&]()
    {
        for (int i = 1; i <= Vt.size() + 1; ++i) cout << setw(8) << setfill('-') << "+";
        cout << '+' << endl;
    };
    cout << setiosflags(ios::left);
    printLine();
    cout << "|\t";
    for (auto vt : Vt) cout << '|' << vt << '\t';
    cout << '|' << endl;
    printLine();
    for (auto vn : Vn)
    {
        cout << '|' << vn << '\t';
        for (auto vt : Vt)
        {
            if (parTable.count({ vn,vt })) cout << '|' << parTable[{vn, vt}] << '\t';
            else cout << "|\t";
        }
        cout << '|' << endl;
        printLine();
    }
    cout << resetiosflags(ios::left);
    return;
}

int SyntacticParser::analyze(string sentence)
{
    sentence += '#';
    stack<char> s;
    s.push('#');
    s.push(start);
    for (int i = 0; i < sentence.size(); ++i)
    {
        while (isVn(s.top()))
        {
            if (parTable.count({ s.top(),sentence[i] }) == 0) return i; //分析失败，返回出错位置
            const string& rig = parTable[{ s.top(), sentence[i] }].rig;
            s.pop();
            for (int j = rig.size() - 1; j >= 0; --j) if (rig[j] != 'e') s.push(rig[j]);
        }
        if (s.size() && s.top() == sentence[i]) s.pop();
        else return i; //分析失败，返回出错位置
    }
    return -1;
}

void SyntacticParser::visibleAnalyze(string sentence)
{
    //设置参数
    cout << setiosflags(ios::left) << setfill(' ');
    const int WID1 = 8, WID2 = 20, WID3 = 20, WID4 = 10;

    //初始化
    int ord = 1;
    sentence += '#'; //输入串
    string stk = "#"; //用字符串模拟栈便于展示内部状态
    stk += start; //文法起始符入栈

    //辅助函数
    auto printLine = [&]() {for (int i = 1; i <= WID1 + WID2 + WID3 + WID4; ++i) cout << '-'; cout << endl; };
    auto printState = [&](bool f, const Production& p = Production())
    {
        cout << setw(WID1) << ord++;
        cout << setw(WID2) << stk;
        cout << setw(WID3) << sentence;
        if (f) cout << p << endl;
        else cout << endl;
    };

    //打印表头
    cout << "\n开始分析：" << endl;
    printLine();
    cout << setw(WID1) << "序号";
    cout << setw(WID2) << "分析栈";
    cout << setw(WID3) << "输入串";
    cout << setw(WID4) << "产生式";
    cout << endl;
    printLine();

    //开始分析
    bool error = 0; //是否报错
    while (sentence.size() && !error) //输入串未读完且无报错则继续
    {
        printState(0); //打印信息
        while (isVn(stk.back())) //栈顶为非终结符
        {
            if (parTable.count({ stk.back(), sentence.front() }) == 0) //分析表没找到对应产生式
            {
                error = 1; //报错
                break; //退出
            }
            const Production& p = parTable[{ stk.back(), sentence.front() }]; //要使用的产生式
            stk.pop_back(); //弹出栈顶
            for (int j = p.rig.size() - 1; j >= 0; --j) if (p.rig[j] != 'e') stk.push_back(p.rig[j]); //产生式右侧反向入栈
            printState(1, p); //打印信息
        }
        if (stk.size() && stk.back() == sentence.front()) //栈顶为终结符且与当前输入字符相同
        {
            sentence.erase(sentence.begin());
            stk.pop_back();
        }
        else error = 1; //否则报错
    }
    printLine();

    //输出分析结果
    cout << "分析结果：" << (error ? "语句错误！" : "语句正确！") << endl;

    //参数复原
    cout << resetiosflags(ios::left);
    return;
}
/*-----------------------------------------------------------------------*/
int main()
{
    Grammar g; //根据输入构造文法
    SyntacticParser sp(g); //根据文法构造分析程序

    //测试分析程序
    int cnts = 0;
    string s;
    cout << "\n请输入需要分析的语句数量：" << endl;
    cin >> cnts;
    while (cnts--)
    {
        cout << "\n请输入要分析的语句：" << endl;
        cin >> s;
        sp.visibleAnalyze(s);
    }
    return 0;
}
/*-----------------------------------------------------------------------*/
/*------------------------------------------------------------------------
测试用例
1、不含空字
3
Z→aBa
B→c
B→bB

正确：abbbbca
正确：aca
错误：abbba
错误：aa

2、含空字
8
E→TA
A→+TA
A→e
T→FB
B→*FB
B→e
F→(E)
F→i

正确：(i*i+i)*i
正确：(i)
错误：i+i*()
错误：ii

------------------------------------------------------------------------*/
