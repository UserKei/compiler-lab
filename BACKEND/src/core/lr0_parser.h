#ifndef LR0_PARSER_H
#define LR0_PARSER_H

#include <string>
#include <vector>
#include <map>

namespace LR0Parser {
    // 语法读取和处理
    void readGrammarFromString(const std::string& grammarContent);
    void readGrammarFromFile(const std::string& filename);
    
    // 获取产生式信息
    std::vector<std::string> getProductionLeftSides();
    std::vector<std::vector<std::string>> getProductionRightSides();
    
    // 解析相关
    struct ParseResult {
        bool success;
        std::string message;
        std::vector<std::string> parseSteps;
        std::map<std::string, std::vector<std::string>> productions;
    };
    
    ParseResult parseInput(const std::string& input);
    
    // 调试和信息输出
    void printGrammar();
    void printAutomaton();
}

#endif // LR0_PARSER_H
