/* ����ԭ�����ҵ4-LL(1)������ ��һС�� ������ ��Ȼ ë�迥 ��ܰ */
#include<iostream>
#include<vector>
#include<map>
#include<set>
#include<string>
#include<iomanip>
#include<stack>
using namespace std;

//#define debug

inline bool isVn(char ch) { return ch >= 'A' && ch <= 'Z'; } //�ж��Ƿ���ս��

class SyntacticParser; //��ǰ����
/*-----------------------------------------------------------------------*/
struct Production
{
    char lef; //����ʽ�����ս��
    string rig; //����ʽ�Ҳ�

    //��������
    friend istream& operator >> (istream& in, Production& p)
    {
        string read;
        in >> read;
        p.lef = read.front();
        p.rig = read.substr(3);
        return in;
    }

    //�������
    friend ostream& operator << (ostream& out, const Production& p)
    {
        out << p.lef << "��" << p.rig; 
        return out;
    }
};
/*-----------------------------------------------------------------------*/
class Grammar
{
    friend class SyntacticParser;

protected:
    vector<Production> prods; //����ʽ
    map<char, vector<int>> to; //ÿ�����ս����Ӧ�Ĳ���ʽ���

    void showTips() const; //��ʾ��ʾ��Ϣ
    void input(); //����
public:
    Grammar(); //�������빹���ķ�
    void print(); //���
};

void Grammar::showTips() const
{
    cout << setiosflags(ios::left);
    cout << "+------------------------------------------------+" << endl;
    cout << "|" << setw(48) << "�ķ�����ע�����" << '|' << endl;
    cout << "|" << setw(48) << "1�������ս�������ս�����õ���Ӣ����ĸ��ʾ" << '|' << endl;
    cout << "|" << setw(48) << "2���ս����Сд��ĸ��ʾ�����ս���ô�д��ĸ��ʾ" << '|' << endl;
    cout << "|" << setw(48) << R"(3������epsilon��"e"��ʾ)" << '|' << endl;
    cout << "|" << setw(48) << R"(4����ͷ��"��"��"->"��ʾ")" << '|' << endl;
    cout << "|" << setw(48) << "5��һ������ʽ�������ַ��䲻���ո�" << '|' << endl;
    cout << "|" << setw(48) << "6��������ķ�Ӧ����LL(1)�ķ�Ҫ��" << '|' << endl;
    cout << "|" << setw(48) << "7������ĵ�һ������ʽӦΪ�ķ���ʼ���Ĳ���ʽ" << '|' << endl;
    cout << "+------------------------------------------------+" << endl;
    cout << resetiosflags(ios::left);
    return;
}

void Grammar::input()
{
    int cntp = 0;
    Production p;
    cout << "\n�������ķ�����ʽ������" << endl;
    cin >> cntp;
    cout << "\n���������ʽ��" << endl;
    while (cntp--)
    {
        //�������ʽ
        cin >> p;

        //��Ӳ���ʽ
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
    cout << "\n�ķ�����ʽΪ��" << endl;
    for (auto& p : prods) cout << p << endl;
    return;
}
/*-----------------------------------------------------------------------*/
class SyntacticParser
{
protected:
    set<char> Vn, Vt; //���ս�������ս����
    map<pair<char,char>, Production> parTable; //LL(1)������
    map<char, set<char>> first, follow; //FIRST������FOLLOW����
    map<char, bool> eps; //���ս���Ƿ���Ƶ�Ϊ����
    char start; //�ķ���ʼ��

    void getV(const Grammar& g); //��Vt����Vn��
    void getFirstOf(const Grammar& g, const char c); //��FIRST(c)
    void getFirst(const Grammar& g); //�����з��ս����FIRST����
    void getFollow(const Grammar& g); //�����з��ս����FOLLOW����
    void getParsingTable(const Grammar& g); //��LL(1)������

public:
    SyntacticParser(const Grammar& g); //�����ķ�g�����﷨������
    void showFirst() const; //������з��ս����FIRST����
    void showFollow() const; //������з��ս����FOLLOW����
    void printParsingTable(); //��ӡLL(1)������
    int analyze(string sentence); //�������
    void visibleAnalyze(string sentence); //������䲢��ʾ��������
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
    for (auto pid : g.to.find(c)->second) //����c�Ĳ���ʽid
    {
        const string& rig = g.prods[pid].rig; //����ʽ�Ҳ��ַ���
        for (auto v : rig) //��������ʽ�Ҳ������ַ�
        {
            if (isVn(v)) //v�Ƿ��ս��
            {
                if (first[v].empty()) //FIRST(v)��û��
                {
                    getFirstOf(g, v); //�ݹ����FIRST(v)��������ݹ��򲻻���ѭ����
                }
                for (auto e : first[v]) //��FIRST(v)-{e}����FIRST(c)
                {
                    if (e != 'e') first[c].insert(e); //�������ⶼ����
                }
            }
            else //v���ս��
            {
                first[c].insert(v); //��v����FIRST(c)
                if (v == 'e') eps[c] = 1; //v�ǿ��֣����c
            }
            if (!eps[v]) break; //FIRST(v)�������֣��˳�ѭ��
        }
    }
    return;
}

void SyntacticParser::getFirst(const Grammar& g)
{
    //�������ս��������δ��FIRST��������FIRST����
    for (auto v : Vn) if (first[v].empty()) getFirstOf(g, v); 
    return;
}

void SyntacticParser::getFollow(const Grammar& g)
{   
    //�ķ���ʼ����FOLLOWԪ���м��������"#"
    follow[start].insert('#'); 

    while (1)
    {
        //����һ�ֲ���ǰFOLLOW�����ܴ�С
        int beforeSize = 0;
        for (auto& s : follow) beforeSize += s.second.size();

        //��ʼһ�ֲ���
        for (auto& p : g.prods) //�������в���ʽ
        {
            for (int i = 0; i < p.rig.size(); ++i) //��������ʽ�Ҳ����з���
            {
                const char c = p.rig[i]; //����ʽ�Ҳ�ĳ���ַ�
                if (isVn(c)) //c�Ƿ��ս��
                {
                    bool endFlag = 1; //p.lef�Ƿ����Ƶ���...c
                    for (int j = i + 1; j < p.rig.size(); ++j) //����c�Ҳ��ַ�
                    {
                        const char v = p.rig[j]; //c�Ҳ�ĳ���ַ�
                        if (isVn(v)) //v�Ƿ��ս��
                        {
                            for (auto e : first[v]) //��FIRST(v)-{e}����FOLLOW(c)
                            {
                                if (e != 'e') follow[c].insert(e); //�������ⶼ����
                            }
                            if (!eps[v]) //v�����Ƶ�Ϊ���֣��˳�
                            {
                                endFlag = 0;
                                break;
                            }
                        }
                        else //v���ս��������FOLLOW(c)���˳�
                        {
                            follow[c].insert(v);
                            endFlag = 0;
                            break;
                        }
                    }
                    if (endFlag) //p.lef�����Ƶ���...c
                    {
                        for (auto e : follow[p.lef]) //��FOLLOW(p.lef)����FOLLOW(c)
                        {
                            follow[c].insert(e);
                        }
                    }
                }
            }
        }

        //����һ�ֲ�����FOLLOW�����ܴ�С
        int afterSize = 0;
        for (auto& s : follow) afterSize += s.second.size();

        //FOLLOW���ϲ�������ѭ������
        if (beforeSize == afterSize) break;
    }
    return;
}

void SyntacticParser::getParsingTable(const Grammar& g)
{
    for (auto vn : Vn) //�������ս��
    {
        for (auto vt : Vt) //�����ս��
        {
            if (first[vn].count(vt)) //��vt����FIRST(vn)���Ҳ���ʽ
            {
                for (auto pid : g.to.find(vn)->second) //����vn����ʽid
                {
                    auto& p = g.prods[pid]; //vn��ĳ������ʽ
                    for (auto c : p.rig) //��������ַ����жϲ���ʽ��FIRST�Ƿ����vt
                    {
                        if (isVn(c)) //c�Ƿ��ս��
                        {
                            if (first[c].count(vt)) parTable[{vn, vt}] = p; //��FIRST(c)����vt��pΪ����
                            if (!eps[c]) break; //c�����Ƶ�Ϊ���֣��˳�
                        }
                        else //c���ս��
                        {
                            if (c == vt) parTable[{vn, vt}] = p; //��cΪvt��pΪ����
                            break;
                        }
                    }
                    if (parTable.count({ vn, vt })) break; //�������ʽ�Ѿ��ҵ����˳�ѭ��
                }
            }
            else if (follow[vn].count(vt) && eps[vn]) //vt����FOLLOW(vn)��vn���Ƶ�Ϊ����
            {
                for (auto pid : g.to.find(vn)->second) //����vn����ʽid
                {
                    auto& p = g.prods[pid]; //vn��ĳ������ʽ
                    if (p.rig.front() == 'e') //pΪvn��e
                    {
                        parTable[{vn, vt}] = p; //��p��Ϊvn����vtʱʹ�õĲ���ʽ
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
    cout << "\nFIRST���ϣ�" << endl;
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
    cout << "\nFOLLOW���ϣ�" << endl; 
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
    cout << "\nLL(1)������" << endl;
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
            if (parTable.count({ s.top(),sentence[i] }) == 0) return i; //����ʧ�ܣ����س���λ��
            const string& rig = parTable[{ s.top(), sentence[i] }].rig;
            s.pop();
            for (int j = rig.size() - 1; j >= 0; --j) if (rig[j] != 'e') s.push(rig[j]);
        }
        if (s.size() && s.top() == sentence[i]) s.pop();
        else return i; //����ʧ�ܣ����س���λ��
    }
    return -1;
}

void SyntacticParser::visibleAnalyze(string sentence)
{
    //���ò���
    cout << setiosflags(ios::left) << setfill(' ');
    const int WID1 = 8, WID2 = 20, WID3 = 20, WID4 = 10;

    //��ʼ��
    int ord = 1;
    sentence += '#'; //���봮
    string stk = "#"; //���ַ���ģ��ջ����չʾ�ڲ�״̬
    stk += start; //�ķ���ʼ����ջ

    //��������
    auto printLine = [&]() {for (int i = 1; i <= WID1 + WID2 + WID3 + WID4; ++i) cout << '-'; cout << endl; };
    auto printState = [&](bool f, const Production& p = Production())
    {
        cout << setw(WID1) << ord++;
        cout << setw(WID2) << stk;
        cout << setw(WID3) << sentence;
        if (f) cout << p << endl;
        else cout << endl;
    };

    //��ӡ��ͷ
    cout << "\n��ʼ������" << endl;
    printLine();
    cout << setw(WID1) << "���";
    cout << setw(WID2) << "����ջ";
    cout << setw(WID3) << "���봮";
    cout << setw(WID4) << "����ʽ";
    cout << endl;
    printLine();

    //��ʼ����
    bool error = 0; //�Ƿ񱨴�
    while (sentence.size() && !error) //���봮δ�������ޱ��������
    {
        printState(0); //��ӡ��Ϣ
        while (isVn(stk.back())) //ջ��Ϊ���ս��
        {
            if (parTable.count({ stk.back(), sentence.front() }) == 0) //������û�ҵ���Ӧ����ʽ
            {
                error = 1; //����
                break; //�˳�
            }
            const Production& p = parTable[{ stk.back(), sentence.front() }]; //Ҫʹ�õĲ���ʽ
            stk.pop_back(); //����ջ��
            for (int j = p.rig.size() - 1; j >= 0; --j) if (p.rig[j] != 'e') stk.push_back(p.rig[j]); //����ʽ�Ҳ෴����ջ
            printState(1, p); //��ӡ��Ϣ
        }
        if (stk.size() && stk.back() == sentence.front()) //ջ��Ϊ�ս�����뵱ǰ�����ַ���ͬ
        {
            sentence.erase(sentence.begin());
            stk.pop_back();
        }
        else error = 1; //���򱨴�
    }
    printLine();

    //����������
    cout << "���������" << (error ? "������" : "�����ȷ��") << endl;

    //������ԭ
    cout << resetiosflags(ios::left);
    return;
}
/*-----------------------------------------------------------------------*/
int main()
{
    Grammar g; //�������빹���ķ�
    SyntacticParser sp(g); //�����ķ������������

    //���Է�������
    int cnts = 0;
    string s;
    cout << "\n��������Ҫ���������������" << endl;
    cin >> cnts;
    while (cnts--)
    {
        cout << "\n������Ҫ��������䣺" << endl;
        cin >> s;
        sp.visibleAnalyze(s);
    }
    return 0;
}
/*-----------------------------------------------------------------------*/
/*------------------------------------------------------------------------
��������
1����������
3
Z��aBa
B��c
B��bB

��ȷ��abbbbca
��ȷ��aca
����abbba
����aa

2��������
8
E��TA
A��+TA
A��e
T��FB
B��*FB
B��e
F��(E)
F��i

��ȷ��(i*i+i)*i
��ȷ��(i)
����i+i*()
����ii

------------------------------------------------------------------------*/
