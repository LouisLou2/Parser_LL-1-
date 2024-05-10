#include "parser2.h"

int main(){
    Parser parser;
    parser.init("D:/OneDrive - csu.edu.cn/Codes/parser_LL(1)/test/init.txt");
    parser.analyse();
    return 0;
}