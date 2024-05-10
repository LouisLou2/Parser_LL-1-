#include<stdio.h>
#include<string>
#include<set>
#include<map>
#include<vector>
#include<stack>

using namespace std;

class Parser{
private:
    // fields
    char startC;
    set<char> VtSet; // 终结符集合
    set<char> VnSet; // 非终结符集合
    map<char, set<string>> production; // 产生式集合, key是非终结符，value是对应的产生式
    map<char, char> VtFirstSet;
    map<char, set<char>> VnFirstSet; // first集合
    map<char, set<char>> followSet; // follow集合
    map<pair<char, char>, set<string>::iterator> predict_map;// 分析表，这里pair是非终结符和终结符的组合，这里iterator是指向set中存储的string的迭代器
    vector<string> testStrs;
    // 分析需要的栈
    stack<char> anaStack;
    stack<char> inputStack;
    // util field 这些字段不是语法必须的，只是为了在分析过程中复用的数据结构
    vector<char> VnVec;
    set<char> temp_firstSet;
    string vstr;
    // functions
    bool isVt(const char& c);
    bool isVn(const char& c);
    bool isEpsilon(const string& right);
    bool nullable(const set<char>& aFirstSet);
    void eliminate_left_recursion();
    void solveImmediateLR(const char vn); // 处理直接左递归
    void solveNonImmediateLR(const char vn1, const char vn2);// 将间接左递归变为直接左递归
    void resolve_first_set(); // 求解FIRST集合
    void resolve_follow_set(); // 求解FOLLOW集合
    void resolve_first_set(const string& right);
    void make_prediction_table();

    // print_funcs:
    void print_named_divider(const char* name);
    void print_grammar_basic();
    void print_productions();
    void print_prediction_table();
    void visualStr(const string& str);
    void visualStr(const char& c);
    void print_visual(const char& c,int len);
    void print_analyse_snapshot();
    void print_set(bool kind);
    void print_first_set();
    void print_follow_set();
public:
    // functions
    void init(const char* filePath);
    void init_syntax_collection(const char* filePath);
    void analyse();
};