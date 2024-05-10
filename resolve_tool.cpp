#include <stdio.h>
#include <cstdlib>
#include "resolve_tool.h"

void processManage(LogLevel level, const char* message){
    if(level == ERROR){
        printf("[ERROR]: %s\n", message);
        std::exit(1);
    }
    printLog(level, message);
}

void printLog(LogLevel level, const char* message){
    switch(level){
        case INFO:
            printf("[INFO]: %s\n", message);
            break;
        case WARNING:
            printf("[WARNING]: %s\n", message);
            break;
        case ERROR:
            printf("[ERROR]: %s\n", message);
            break;
    }
}