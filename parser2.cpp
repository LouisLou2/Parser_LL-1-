#define RESERVATION_NUM 50
#include<fstream>
#include<stdio.h>

#include "defination.h"
#include "parser2.h"
#include "resolve_tool.h"

using namespace std;

void Parser::init(const char* filePath){
    init_syntax_collection(filePath);
    print_named_divider("Grammar Basic");
    print_grammar_basic();

    eliminate_left_recursion();
	print_named_divider("Grammar Basic After Eliminating Left Recursion");
	print_grammar_basic();

	resolve_first_set();
	resolve_follow_set();
    print_named_divider("First Set and Follow Set");
    print_first_set();
    print_follow_set();

    make_prediction_table();
    print_named_divider("Prediction Table");
    print_prediction_table();
};

/*
按照以下规则:

所有符号都只占一个字符
第0行是非终结符(ASCII:[65-90])，空格分隔
第1行是终结符(ASCII:except[65-90] except [0-31])，空格分隔
接下来从第2行开始每行一个产生式
隔一个空行
接下来n行时测试用例
*/
void Parser::init_syntax_collection(const char* filePath){
    ifstream file_input(filePath);
    if(!(file_input.is_open())){
        processManage(LogLevel::ERROR, "File not found");
        //这里已经会将整个程序终止，不需要再return
    }
    char c;
    unsigned short line = 0;
    VtSet.insert(EPSILON);
    VtSet.insert(END_CHAR);
    file_input.get(c);
    startC = c;
    VnSet.insert(c);
    // 读取终结符和非终结符
    while(true){
        if(line==2) break;
        file_input.get(c);
        if(c==BLANK)continue;
        if(c=='\n'){
            ++line;
            continue;
        }
        if(line==0){
            VnSet.insert(c);
        }else if(line==1){
            VtSet.insert(c);
        }
    }
    bool lastNewLine=false;
    // 读取产生式
    while(true){
        file_input.get(c);
        char vn = c;
        if(lastNewLine&&c=='\n') break;
        if(c=='\n'){
            ++line;
            lastNewLine=true;
            continue;
        }
        if(c==BLANK)continue;
        for(int i=0;i<2;++i) file_input.get(c);// 读过“->”

        if (production.find(vn) == production.end()) {
			production[vn]=set<string>();
		}
        auto &new_prods=production[vn];
        bool newVt=true;
        while(newVt){
            string p_str;
            while(true){
                file_input.get(c);
                if(c==OR_CHAR) break;
                if (c == '\n') {
                    newVt = false;
                    lastNewLine = true;
                    break;
                }
                p_str.push_back(c);
            }
            new_prods.insert(p_str);
        }
    }
    //读取测试字符串
    bool hasChar=true;
    while(hasChar){
        string testStr;
        while(true){
            file_input.get(c);
            if(file_input.eof()) {
                hasChar=false;
                break;
            }
            if(c=='\n') break;
            testStr.push_back(c);
        }
        testStrs.push_back(testStr);
    }
};

void Parser::eliminate_left_recursion() {
    VnVec.assign(VnSet.begin(), VnSet.end());// 为了保证遍历不出错，不能使用set, set总是动态排序
    for (int i = 0; i < VnVec.size(); ++i) {
        /*for (int j = 0; j < i; ++j) {
            solveNonImmediateLR(VnVec[i], VnVec[j]);
        }*/
        solveImmediateLR(VnVec[i]);
    }
    VnSet.insert(VnVec.begin(), VnVec.end());
};

void Parser::solveImmediateLR(const char vn) {
    char newVn = vn - 65;
    vector<string> alphas, betas;
    auto& prods = production[vn];

    for (const auto& prod : prods) {
        if (prod.at(0) == vn) {
            alphas.push_back(prod.substr(1));
        }
        else {
            betas.push_back(prod);
        }
    }
    if (alphas.empty()) return;
    prods.clear();
    VnVec.push_back(newVn);
    production[newVn]=set<string>();
    auto& new_prods = production[newVn];
    // no left recursion
    // no beta
    if (betas.empty()) prods.insert(string(1,newVn));
    for (const string& beta : betas) {
        prods.insert(beta + newVn);
    }
    for (const string& alpha : alphas) {
        new_prods.insert(alpha + newVn);
    }
    new_prods.insert(string(1,EPSILON));
};

void Parser::solveNonImmediateLR(const char vn1, const char vn2) {
    auto vn1_prods = production[vn1];
    auto vn2_prods = production[vn2];
    production[vn1] = set<string>();
    auto& new_vn1_prods = production[vn1];
    for (const auto& prod : vn1_prods) {
        if (prod.at(0) == vn2) {
            for (const auto& prod2 : vn2_prods) {
                new_vn1_prods.insert(prod2 + prod.substr(1));
            }
        }
        else {
            new_vn1_prods.insert(prod);
        }
    }
};

bool Parser::isVt(const char& c) {
    return !isVn(c);
}

bool Parser::isVn(const char& c) {
    return c >= 'A' && c <= 'Z'|| c >= 0 && c <= 25;
}

bool Parser::isEpsilon(const string& right) {
    return right.size() == 1 && right.at(0) == EPSILON;
}

bool Parser::nullable(const set<char>& aFirstSet) {
    return aFirstSet.contains(EPSILON);
}

void ::Parser::resolve_first_set(const string& right) {
    temp_firstSet.clear();
    // 此函数修改temp_firstSet作为结果
    if (isEpsilon(right) || isVt(right[0])) {
        temp_firstSet.insert(right[0]);
        return;
    }
	bool next = true;// 是否还要看下一个非终结符
	int strInd = 0;
	while (next && strInd < right.size()) {
		next = false;
		char Y = right[strInd];
		if (isVt(Y)) {
			if (Y != EPSILON) temp_firstSet.insert(VtFirstSet[Y]);// 其实就是直接添加Y
			else next = true;
            ++strInd;
			continue;
		}
		set<char>& firstY = VnFirstSet[Y];
		for (auto it = firstY.begin(); it != firstY.end(); ++it) {
			temp_firstSet.insert(*it);
			if (*it == EPSILON) next = true;
		}
        ++strInd;
	}
    //if (next&&!right.empty()) temp_firstSet.insert(EPSILON);
}
void Parser::resolve_first_set() {
    for (const auto& c : VtSet) {
        VtFirstSet[c] = c;
    }
    bool change = true;
    while (change) {
        change = false;
		for (const auto& [vn, prod] : production) {
			set<char>& firstX = VnFirstSet[vn];

			for (const auto& p_str : prod) {
				// 这里是非终结符为vn, p_str为右部式子
				if (isEpsilon(p_str) || isVt(p_str[0])) firstX.insert(p_str[0]);
				else {
					// 当前是非终结符的情况
					bool next = true;// 是否还要看下一个非终结符
					int strInd = 0;
					while (next && strInd < p_str.size()) {
						next = false;
						char Y = p_str[strInd];
                        if (isVt(Y)) {
                            if (Y != EPSILON) firstX.insert(VtFirstSet[Y]);// 其实就是直接添加Y
                            else next = true;
                            ++strInd;
                            continue;
                        }
						set<char>& firstY = VnFirstSet[Y];
						for (auto it = firstY.begin(); it != firstY.end(); ++it) {
                            if (firstX.find(*it) == firstX.end()) {
								firstX.insert(*it);
								change = true;
                            }
							else if (*it == EPSILON) next = true;
						}
                        ++strInd;
					}
					if (next) firstX.insert(EPSILON);
				}
			}
		}
    }
}

void Parser::resolve_follow_set() {
	char c;
	followSet[startC].insert(END_CHAR);
	bool change = true;
	while (change) {
		change = false;
		for (const auto& [vn, prod] : production) {
			for (const auto& p_str : prod) {
				// 这里是非终结符为vn, p_str为右部式子
				for (int i = 0; i < p_str.size(); ++i) {
					c = p_str.at(i);
					if (isVt(c))continue;

                    set<char>& CFollow = followSet[c];

					resolve_first_set(p_str.substr(i + 1));
                    set<char>& alphaFS = temp_firstSet;

					for (auto it = alphaFS.begin(); it != alphaFS.end(); ++it) {
						if (*it == EPSILON)continue;
                        if (CFollow.find(*it) == CFollow.end()) {
                            CFollow.insert(*it);
                            change = true;
                        }
					}
                    if (nullable(alphaFS) || i == p_str.size() - 1) {
                        set<char>& leftFollow = followSet[vn];
                        for (auto it = leftFollow.begin(); it != leftFollow.end(); ++it) {
                            if (CFollow.find(* it) == CFollow.end()) {
                                CFollow.insert(*it);
                                change = true;
                            }
                        }
                    }

				}
			}
		}
	}
}

void Parser::make_prediction_table() {
	for (const auto& [vn, prods] : production) {
        set<char>& VnFollow = followSet[vn];
        for (auto it = prods.begin(); it != prods.end(); ++it) {
			// 这里是非终结符为vn, p_str为右部式子
			resolve_first_set(*it);
			set<char>& alphaFS = temp_firstSet;
            for (auto c_it : alphaFS) predict_map.insert({ { vn, c_it }, it });
            if (!alphaFS.contains(EPSILON)) continue;
            for (auto c_it : VnFollow)  predict_map.insert({ { vn, c_it }, it });
        }
	}
}

void Parser::visualStr(const string& str) {
    vstr.clear();
    for (const auto&c:str) {
        if (c >= 0 && c <= 25) {
            vstr.push_back(char(c + 65));
            vstr.push_back('\'');
            continue;
        }
        vstr.push_back(c);
    }
}
void Parser::visualStr(const char& c) {
	vstr.clear();
	if (c >= 0 && c <= 25) {
		vstr.push_back(char(c + 65));
		vstr.push_back('\'');
	}
	vstr.push_back(c);
}

void Parser::print_visual(const char& c,int len) {
	if (c >= 0 && c <= 25) {
		printf("%c", char(c + 65));
		printf("%c", '\'');
        for(int i=0;i<len-2;++i) printf("%c", ' ');
        return;
	}
    printf("%c", c);
    for (int i = 0; i < len - 1; ++i) printf("%c", ' ');
}

void Parser::print_prediction_table() {
    printf("\n%s\n", "Prediction Table:");
    printf("%-7c", ' ');
	for (const auto vt : VtSet) {
        if (vt == EPSILON)continue;
        printf("%-7c", vt);
	}
    printf("%c", '\n');
    for (auto vn : VnSet) {
        auto it = predict_map.begin();
        visualStr(vn);
        printf("%-8s",vstr.c_str());
        for (auto vt : VtSet) {
            if (vt == EPSILON)continue;
            it = predict_map.find({ vn,vt });
            if(it==predict_map.end()) printf("%-7c", ' ');
            else {
                visualStr(*(it->second));
                printf("%-7s", vstr.c_str());
            }
        }
        printf("%c", '\n');
    }
}

void Parser::print_analyse_snapshot() {
    uint16_t maxlen = 20;
    uint16_t len = 0;
    auto anaCon = anaStack._Get_container();
    auto inputCon = inputStack._Get_container();
    printf("%-17s", "analyse stack:");
    for (const auto& c : anaCon) {
		if (c >= 0 && c <= 25) {
            printf("%c", char(c + 65));
            printf("%c", '\'');
            len += 2;
            continue;
		}
        printf("%c", c);
        ++len;
    }
    for (int i = 0; i < maxlen - len; ++i) printf("%c", ' ');
    printf("%-20s", "  input stack:");
	for (const auto& c : inputCon) {
		if (c >= 0 && c <= 25) {
			printf("%c", char(c + 65));
			printf("%c", '\'');
			continue;
		}
		printf("%c", c);
	}
    printf("%c", '\n');
}

void Parser::analyse() {
    print_named_divider("Analyse");
    auto it = predict_map.begin();
    set<string>::iterator strIter;
    for (const auto& str : testStrs) {
        printf("%s %s\n", "Test Instance:", str.c_str());
        if (!anaStack.empty()) anaStack = stack<char>();
        if (!inputStack.empty()) inputStack = stack<char>();
		anaStack.push(END_CHAR);
		anaStack.push(startC);
        inputStack.push(END_CHAR);
        for (int i = str.size() - 1; i >= 0; --i) inputStack.push(str[i]);
        while (!anaStack.empty()) {
            print_analyse_snapshot();
            if (isVt(anaStack.top())) {
                if (anaStack.top() != inputStack.top()) processManage(ERROR, "栈顶终结符不匹配");
                anaStack.pop();
                inputStack.pop();
                continue;
            }
            it = predict_map.find({ anaStack.top(),inputStack.top() });
            if(it==predict_map.end()) processManage(ERROR, "访问分析表中不存在的项");
            anaStack.pop();
            strIter = it->second;
            for (int i = strIter->size() - 1; i >= 0; --i) {
                if (strIter->at(i) == EPSILON)continue;
                anaStack.push(strIter->at(i));
            }
        }
        printf("%c", '\n');
    }
}

void Parser::print_set(bool kind) {
    map<char, set<char>> * uu = &VnFirstSet;
    if (!kind) uu = &followSet;

	if(kind) printf("\n%s\n", "FIRST SET:");
    else printf("\n%s\n", "FOLLOW SET:");

	for (auto it = uu->begin(); it != uu->end(); ++it) {
		print_visual(it->first,3);
		printf("%s", "{ ");
		for (const auto& c : it->second) {
			printf("%c ", c);
		}
		printf("%s", "}\n");
	}
}

void Parser::print_first_set() {
    print_set(true);
}
void Parser::print_follow_set() {
    print_set(false);
}
void Parser::print_grammar_basic() {
    // 打印基本信息
    printf("\n%-15s", "Terminal: ");
    for(const auto&c: VtSet) printf("%c ", c);
    printf("\n%-15s", "Non-terminal: ");
    //for (const auto& c : VnSet) printf("%c ", c);
    for (const auto& [vn, prods] : production) {
        print_visual(vn, 3);
    }
    printf("\n%s%c\n", "Start Symbol:", startC);
    print_productions();
}
void Parser::print_productions() {
	printf("\n%s", "Production:");
	for (const auto& [vn, prods] : production) {
        printf("%c", '\n');
        print_visual(vn, 3);
        printf("%s", "->");
        auto preend = --prods.end();
        auto it = prods.begin();
		while(it != preend) {
            visualStr(*it);
			printf("%s", vstr.c_str());
			printf("%c", '|');
            ++it;
        }
        printf("%s", it->c_str());
	}
}

void Parser::print_named_divider(const char*name) {
    printf("\n------------%s------------\n", name);
}