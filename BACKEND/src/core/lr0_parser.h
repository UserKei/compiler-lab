#ifndef LR0_PARSER_H
#define LR0_PARSER_H

#include <string>
#include <vector>
#include <map>

// 前置声明
namespace ItemSet {
    struct LRItem;
}

namespace LR0Parser {
    // 语法读取和处理
    void readGrammarFromString(const std::string& grammarContent);
    void readGrammarFromFile(const std::string& filename);
    
    // 获取产生式信息
    std::vector<std::string> getProductionLeftSides();
    std::vector<std::vector<std::string>> getProductionRightSides();
    
    // 解析步骤结构
    struct ParseStep {
        int step;
        std::string stateStack;
        std::string symbolStack;
        std::string remainingInput;
        std::string action;
    };
    
    // 分析表行结构
    struct ParseTableRow {
        int state;
        std::map<std::string, std::string> actions;
        std::map<std::string, int> gotos;
    };
    
    // 分析表结构
    struct ParseTable {
        std::vector<std::string> headers;
        std::vector<ParseTableRow> rows;
    };
    
    // 单个产生式结构（包含序号）
    struct Production {
        int index;
        std::string leftSide;
        std::vector<std::string> rightSide;
    };

    // 解析相关
    struct ParseResult {
        bool success;
        std::string message;
        std::vector<ParseStep> parseSteps;
        ParseTable parseTable;
        std::string dotFile;
        bool isAccepted;
        std::map<std::string, std::vector<std::vector<std::string>>> productions; // 保留原有格式用于兼容
        std::vector<Production> productionList; // 新增：按序号排列的产生式列表
    };
    
    // 构建解析表（独立于输入分析）
    ParseResult buildParseTable();
    
    // 解析输入字符串（使用已构建的解析表）
    ParseResult parseInput(const std::string& input);
    
    // 生成DOT文件
    std::string generateDotFile();
    
    // 生成DOT文件内容的辅助函数
    std::string generateDotFileContent(const std::vector<std::vector<ItemSet::LRItem>>& canonicalCollection,
                                     const std::vector<std::map<std::string, std::string>>& actionTable,
                                     const std::vector<std::map<std::string, int>>& gotoTable);
    
    // 调试和信息输出
    void printGrammar();
    void printAutomaton();
}

#endif // LR0_PARSER_H
