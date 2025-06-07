#ifndef REGEX_AUTOMATA_H
#define REGEX_AUTOMATA_H

#include <string>

namespace RegexAutomata {
    // 自动机构建
    bool buildFromRegex(const std::string& regex);
    
    // 字符串匹配
    bool matchString(const std::string& input);
    
    // 获取自动机描述
    std::string getNFADescription();
    std::string getDFADescription();
    
    // 调试和信息输出
    void printNFA();
    void printDFA();
    void printMinimizedDFA();
    
    // 自动机状态管理
    void reset();
    bool isBuilt();
}

#endif // REGEX_AUTOMATA_H
