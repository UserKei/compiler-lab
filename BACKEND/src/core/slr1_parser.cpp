#include "slr1_parser.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>

// 语法解析命名空间 - 用于读取和解析语法文件
namespace Grammar_SLR1 {
    // 非终结符、终结符和所有符号的集合
    std::vector<std::string> nonterminalSymbols;
    std::vector<std::string> terminalSymbols;
    std::vector<std::string> allSymbols;

    // 产生式的左部和右部
    std::vector<std::string> productionLeftSides;
    std::vector<std::vector<std::string>> productionRightSides;

    // 去除字符串首尾的空白字符
    std::string trimWhitespace(const std::string& inputString) {
        size_t startPos = inputString.find_first_not_of(" \t\r\n");
        size_t endPos = inputString.find_last_not_of(" \t\r\n");
        return (startPos == std::string::npos) ? "" : inputString.substr(startPos, endPos - startPos + 1);
    }

    // 判断字符串是否为非终结符
    bool isNonterminal(const std::string& symbol) {
        return !symbol.empty() && std::isupper(symbol[0]);
    }

    // 解析产生式右部
    std::vector<std::string> parseRightHandSide(const std::string& rhs) {
        std::vector<std::string> symbols;
        std::string currentSymbol;

        for (size_t i = 0; i < rhs.size(); ++i) {
            char currentChar = rhs[i];

            // 跳过空格 - 空格作为符号分隔符
            if (std::isspace(currentChar)) {
                if (!currentSymbol.empty()) {
                    symbols.push_back(currentSymbol);
                    currentSymbol.clear();
                }
                continue;
            }

            // 引号内的符号作为一个整体处理
            if (currentChar == '\'' || currentChar == '"') {
                char quoteChar = currentChar;
                currentSymbol += currentChar;
                ++i;

                // 查找匹配的引号
                while (i < rhs.size() && rhs[i] != quoteChar) {
                    currentSymbol += rhs[i];
                    ++i;
                }

                if (i < rhs.size()) {
                    currentSymbol += quoteChar;
                }
            } else {
                currentSymbol += currentChar;
            }
        }

        // 添加最后一个符号
        if (!currentSymbol.empty()) {
            symbols.push_back(currentSymbol);
        }

        return symbols;
    }

    // 解析语法字符串
    void parseGrammar(const std::string& grammarContent) {
        // 清空现有数据
        nonterminalSymbols.clear();
        terminalSymbols.clear();
        allSymbols.clear();
        productionLeftSides.clear();
        productionRightSides.clear();

        // 临时存储原始产生式规则
        std::vector<std::pair<std::string, std::string>> rawProductionRules;
        std::string startSymbol;
        bool hasStartSymbolS = false;

        std::istringstream grammarStream(grammarContent);
        std::string currentLine;
        int lineNumber = 0;

        while (std::getline(grammarStream, currentLine)) {
            lineNumber++;
            currentLine = trimWhitespace(currentLine);

            // 跳过空行和注释行
            if (currentLine.empty() || currentLine[0] == '#') {
                continue;
            }

            // 查找产生式分隔符 " -> "
            size_t arrowPosition = currentLine.find(" -> ");
            if (arrowPosition == std::string::npos) {
                continue;
            }

            // 提取产生式左部和右部
            std::string leftSide = trimWhitespace(currentLine.substr(0, arrowPosition));
            std::string rightSideStr = trimWhitespace(currentLine.substr(arrowPosition + 4));

            if (leftSide.empty() || rightSideStr.empty()) {
                continue;
            }

            // 记录第一个产生式的左部作为开始符号
            if (rawProductionRules.empty()) startSymbol = leftSide;
            if (leftSide == "S") hasStartSymbolS = true;

            rawProductionRules.emplace_back(leftSide, rightSideStr);
        }

        if (rawProductionRules.empty()) {
            throw std::runtime_error("No valid production rules found in grammar");
        }

        // 总是添加拓广文法的开始符号
        if (!startSymbol.empty()) {
            std::string augmentedStartSymbol = startSymbol + "'";
            rawProductionRules.insert(rawProductionRules.begin(), { augmentedStartSymbol, startSymbol });
            std::cout << "SLR1: Added augmented start production: " << augmentedStartSymbol << " -> " << startSymbol << std::endl;
        }

        // 处理每个产生式
        std::set<std::string> nonterminalSet;
        for (const auto& productionPair : rawProductionRules) {
            nonterminalSet.insert(productionPair.first);
            
            // 支持半角和全角竖线分隔符
            std::string rightSide = productionPair.second;
            // 替换全角竖线为半角竖线
            size_t pos = 0;
            while ((pos = rightSide.find("｜", pos)) != std::string::npos) {
                rightSide.replace(pos, 3, "|");  // 全角竖线是3字节UTF-8
                pos += 1;
            }
            
            std::stringstream stringStream(rightSide);
            std::string alternativeRule;

            // 处理每个可选项（用|分隔）
            while (std::getline(stringStream, alternativeRule, '|')) {
                alternativeRule = trimWhitespace(alternativeRule);
                if (alternativeRule.empty()) continue;

                std::vector<std::string> symbols = parseRightHandSide(alternativeRule);

                if (symbols.empty()) continue;

                productionLeftSides.push_back(productionPair.first);
                productionRightSides.push_back(symbols);
            }
        }

        // 分析符号并分类
        std::set<std::string> allSymbolsSet;
        std::set<std::string> terminalSet;

        // 收集所有右部符号
        for (const std::vector<std::string>& rightSide : productionRightSides) {
            for (const std::string& symbol : rightSide) {
                if (symbol != "epsilon") {
                    allSymbolsSet.insert(symbol);
                    // 如果不是非终结符，则为终结符
                    if (nonterminalSet.find(symbol) == nonterminalSet.end()) {
                        terminalSet.insert(symbol);
                    }
                }
            }
        }

        // 构建非终结符和终结符集合
        nonterminalSymbols.assign(nonterminalSet.begin(), nonterminalSet.end());
        terminalSymbols.assign(terminalSet.begin(), terminalSet.end());
        terminalSymbols.push_back("$");  // 添加结束符

        // 构建所有符号集合
        allSymbols = nonterminalSymbols;
        allSymbols.insert(allSymbols.end(), terminalSymbols.begin(), terminalSymbols.end());
    }
}

// 项目集合处理命名空间
namespace ItemSet_SLR1 {
    // LR项目结构体
    struct LRItem {
        int productionIndex;    // 产生式索引
        int dotPosition;        // 点的位置
        
        bool operator<(const LRItem& other) const {
            if (productionIndex != other.productionIndex) {
                return productionIndex < other.productionIndex;
            }
            return dotPosition < other.dotPosition;
        }
        
        bool operator==(const LRItem& other) const {
            return productionIndex == other.productionIndex && dotPosition == other.dotPosition;
        }
    };

    // 闭包计算
    std::set<LRItem> computeClosure(const std::set<LRItem>& itemSet) {
        std::set<LRItem> closure = itemSet;
        bool hasChanges = true;

        while (hasChanges) {
            hasChanges = false;
            std::set<LRItem> newItems;

            for (const LRItem& item : closure) {
                // 检查点后是否还有符号
                if (item.dotPosition < Grammar_SLR1::productionRightSides[item.productionIndex].size()) {
                    std::string nextSymbol = Grammar_SLR1::productionRightSides[item.productionIndex][item.dotPosition];

                    // 如果点后的符号是非终结符
                    if (Grammar_SLR1::isNonterminal(nextSymbol)) {
                        // 为该非终结符的所有产生式添加项目
                        for (int i = 0; i < Grammar_SLR1::productionLeftSides.size(); ++i) {
                            if (Grammar_SLR1::productionLeftSides[i] == nextSymbol) {
                                LRItem newItem = {i, 0};
                                if (closure.find(newItem) == closure.end() && newItems.find(newItem) == newItems.end()) {
                                    newItems.insert(newItem);
                                    hasChanges = true;
                                }
                            }
                        }
                    }
                }
            }

            closure.insert(newItems.begin(), newItems.end());
        }

        return closure;
    }

    // GOTO函数计算
    std::set<LRItem> computeGoto(const std::set<LRItem>& itemSet, const std::string& symbol) {
        std::set<LRItem> gotoSet;

        for (const LRItem& item : itemSet) {
            // 检查点后是否有符号且该符号匹配
            if (item.dotPosition < Grammar_SLR1::productionRightSides[item.productionIndex].size()) {
                std::string nextSymbol = Grammar_SLR1::productionRightSides[item.productionIndex][item.dotPosition];
                if (nextSymbol == symbol) {
                    LRItem newItem = {item.productionIndex, item.dotPosition + 1};
                    gotoSet.insert(newItem);
                }
            }
        }

        return computeClosure(gotoSet);
    }

    // 规范项目集族计算
    std::vector<std::set<LRItem>> computeCanonicalCollection() {
        std::vector<std::set<LRItem>> collection;

        // 初始项目集 - 假设第一个产生式是增广文法的开始产生式
        std::set<LRItem> initialSet;
        initialSet.insert({0, 0});
        std::set<LRItem> initialClosure = computeClosure(initialSet);
        collection.push_back(initialClosure);

        bool hasChanges = true;
        while (hasChanges) {
            hasChanges = false;
            std::vector<std::set<LRItem>> newSets;

            for (const std::set<LRItem>& itemSet : collection) {
                // 为每个符号计算GOTO
                for (const std::string& symbol : Grammar_SLR1::allSymbols) {
                    std::set<LRItem> gotoSet = computeGoto(itemSet, symbol);
                    
                    if (!gotoSet.empty()) {
                        // 检查该项目集是否已存在
                        bool found = false;
                        for (const std::set<LRItem>& existingSet : collection) {
                            if (existingSet == gotoSet) {
                                found = true;
                                break;
                            }
                        }
                        for (const std::set<LRItem>& existingSet : newSets) {
                            if (existingSet == gotoSet) {
                                found = true;
                                break;
                            }
                        }

                        if (!found) {
                            newSets.push_back(gotoSet);
                            hasChanges = true;
                        }
                    }
                }
            }

            collection.insert(collection.end(), newSets.begin(), newSets.end());
        }

        return collection;
    }
}

// SLR1分析器实现
namespace SLR1Parser {
    // 全局变量
    std::vector<std::set<ItemSet_SLR1::LRItem>> canonicalCollection;
    std::vector<std::map<std::string, std::string>> actionTable;
    std::vector<std::map<std::string, int>> gotoTable;
    std::map<std::string, std::set<std::string>> firstSets;
    std::map<std::string, std::set<std::string>> followSets;

    // 从字符串读取语法
    void readGrammarFromString(const std::string& grammarContent) {
        Grammar_SLR1::parseGrammar(grammarContent);
        
        // 重新计算项目集族和分析表
        canonicalCollection = ItemSet_SLR1::computeCanonicalCollection();
        firstSets = computeFirstSets();
        followSets = computeFollowSets();
        
        // 构建SLR1分析表
        actionTable.clear();
        gotoTable.clear();
        actionTable.resize(canonicalCollection.size());
        gotoTable.resize(canonicalCollection.size());

        // 构建ACTION和GOTO表
        for (int i = 0; i < canonicalCollection.size(); ++i) {
            const std::set<ItemSet_SLR1::LRItem>& itemSet = canonicalCollection[i];

            for (const ItemSet_SLR1::LRItem& item : itemSet) {
                const std::vector<std::string>& production = Grammar_SLR1::productionRightSides[item.productionIndex];

                if (item.dotPosition < production.size()) {
                    // 移入项目
                    std::string nextSymbol = production[item.dotPosition];
                    std::set<ItemSet_SLR1::LRItem> gotoSet = ItemSet_SLR1::computeGoto(canonicalCollection[i], nextSymbol);

                    // 查找GOTO集合的状态编号
                    for (int j = 0; j < canonicalCollection.size(); ++j) {
                        if (canonicalCollection[j] == gotoSet) {
                            if (std::find(Grammar_SLR1::terminalSymbols.begin(), Grammar_SLR1::terminalSymbols.end(), nextSymbol) != Grammar_SLR1::terminalSymbols.end()) {
                                actionTable[i][nextSymbol] = "s" + std::to_string(j);
                            } else {
                                gotoTable[i][nextSymbol] = j;
                            }
                            break;
                        }
                    }
                } else {
                    // 归约项目
                    if (item.productionIndex == 0 && Grammar_SLR1::productionLeftSides[0].find("'") != std::string::npos) {
                        // 接受项目
                        actionTable[i]["$"] = "acc";
                    } else {
                        // 归约项目 - 使用FOLLOW集合
                        std::string leftSide = Grammar_SLR1::productionLeftSides[item.productionIndex];
                        if (followSets.find(leftSide) != followSets.end()) {
                            for (const std::string& followSymbol : followSets[leftSide]) {
                                if (actionTable[i].find(followSymbol) == actionTable[i].end()) {
                                    actionTable[i][followSymbol] = "r" + std::to_string(item.productionIndex);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 从文件读取语法
    void readGrammarFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开语法文件: " + filename);
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        readGrammarFromString(content);
    }

    // 获取产生式左部
    std::vector<std::string> getProductionLeftSides() {
        return Grammar_SLR1::productionLeftSides;
    }

    // 获取产生式右部
    std::vector<std::vector<std::string>> getProductionRightSides() {
        return Grammar_SLR1::productionRightSides;
    }

    // 计算FIRST集合
    std::map<std::string, std::set<std::string>> computeFirstSets() {
        std::map<std::string, std::set<std::string>> first;

        // 初始化
        for (const std::string& terminal : Grammar_SLR1::terminalSymbols) {
            first[terminal].insert(terminal);
        }
        for (const std::string& nonterminal : Grammar_SLR1::nonterminalSymbols) {
            first[nonterminal] = std::set<std::string>();
        }

        bool changed = true;
        while (changed) {
            changed = false;

            for (int i = 0; i < Grammar_SLR1::productionLeftSides.size(); ++i) {
                const std::string& leftSide = Grammar_SLR1::productionLeftSides[i];
                const std::vector<std::string>& rightSide = Grammar_SLR1::productionRightSides[i];

                if (rightSide.size() == 1 && rightSide[0] == "epsilon") {
                    if (first[leftSide].find("epsilon") == first[leftSide].end()) {
                        first[leftSide].insert("epsilon");
                        changed = true;
                    }
                    continue;
                }

                for (int j = 0; j < rightSide.size(); ++j) {
                    const std::string& symbol = rightSide[j];
                    
                    // 添加FIRST(symbol) - {epsilon}
                    for (const std::string& firstSymbol : first[symbol]) {
                        if (firstSymbol != "epsilon" && first[leftSide].find(firstSymbol) == first[leftSide].end()) {
                            first[leftSide].insert(firstSymbol);
                            changed = true;
                        }
                    }

                    // 如果symbol不包含epsilon，停止
                    if (first[symbol].find("epsilon") == first[symbol].end()) {
                        break;
                    }

                    // 如果是最后一个符号且包含epsilon
                    if (j == rightSide.size() - 1) {
                        if (first[leftSide].find("epsilon") == first[leftSide].end()) {
                            first[leftSide].insert("epsilon");
                            changed = true;
                        }
                    }
                }
            }
        }

        return first;
    }

    // 计算FOLLOW集合
    std::map<std::string, std::set<std::string>> computeFollowSets() {
        std::map<std::string, std::set<std::string>> follow;

        // 初始化
        for (const std::string& nonterminal : Grammar_SLR1::nonterminalSymbols) {
            follow[nonterminal] = std::set<std::string>();
        }

        // 开始符号的FOLLOW集合包含$
        if (!Grammar_SLR1::nonterminalSymbols.empty()) {
            follow[Grammar_SLR1::nonterminalSymbols[0]].insert("$");
        }

        bool changed = true;
        while (changed) {
            changed = false;

            for (int i = 0; i < Grammar_SLR1::productionLeftSides.size(); ++i) {
                const std::string& leftSide = Grammar_SLR1::productionLeftSides[i];
                const std::vector<std::string>& rightSide = Grammar_SLR1::productionRightSides[i];

                for (int j = 0; j < rightSide.size(); ++j) {
                    const std::string& symbol = rightSide[j];

                    if (Grammar_SLR1::isNonterminal(symbol)) {
                        // 计算FIRST(beta)，其中beta是symbol后面的符号串
                        if (j + 1 < rightSide.size()) {
                            // 计算后续符号的FIRST集合
                            for (int k = j + 1; k < rightSide.size(); ++k) {
                                const std::string& nextSymbol = rightSide[k];
                                
                                for (const std::string& firstSymbol : firstSets[nextSymbol]) {
                                    if (firstSymbol != "epsilon" && follow[symbol].find(firstSymbol) == follow[symbol].end()) {
                                        follow[symbol].insert(firstSymbol);
                                        changed = true;
                                    }
                                }

                                if (firstSets[nextSymbol].find("epsilon") == firstSets[nextSymbol].end()) {
                                    break;
                                }

                                // 如果是最后一个符号且包含epsilon
                                if (k == rightSide.size() - 1) {
                                    for (const std::string& followSymbol : follow[leftSide]) {
                                        if (follow[symbol].find(followSymbol) == follow[symbol].end()) {
                                            follow[symbol].insert(followSymbol);
                                            changed = true;
                                        }
                                    }
                                }
                            }
                        } else {
                            // symbol在产生式最后
                            for (const std::string& followSymbol : follow[leftSide]) {
                                if (follow[symbol].find(followSymbol) == follow[symbol].end()) {
                                    follow[symbol].insert(followSymbol);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        return follow;
    }

    // 解析输入字符串
    ParseResult parseInput(const std::string& input) {
        ParseResult result;
        result.success = false;
        result.isAccepted = false;

        if (canonicalCollection.empty()) {
            result.message = "语法未初始化";
            return result;
        }

        // 准备输入串
        std::vector<std::string> inputTokens;
        std::istringstream iss(input);
        std::string token;
        while (iss >> token) {
            inputTokens.push_back(token);
        }
        inputTokens.push_back("$");

        // 初始化分析栈
        std::vector<int> stateStack;
        std::vector<std::string> symbolStack;
        stateStack.push_back(0);
        symbolStack.push_back("$");

        int inputIndex = 0;
        int step = 0;

        while (true) {
            step++;
            int currentState = stateStack.back();
            std::string currentSymbol = inputTokens[inputIndex];

            // 记录当前步骤
            ParseStep parseStep;
            parseStep.step = step;
            
            // 构建状态栈字符串
            std::ostringstream stateStackStr;
            for (int state : stateStack) {
                stateStackStr << state << " ";
            }
            parseStep.stateStack = stateStackStr.str();

            // 构建符号栈字符串
            std::ostringstream symbolStackStr;
            for (const std::string& symbol : symbolStack) {
                symbolStackStr << symbol << " ";
            }
            parseStep.symbolStack = symbolStackStr.str();

            // 构建剩余输入字符串
            std::ostringstream remainingInputStr;
            for (int i = inputIndex; i < inputTokens.size(); ++i) {
                remainingInputStr << inputTokens[i] << " ";
            }
            parseStep.remainingInput = remainingInputStr.str();

            // 查找动作
            if (actionTable[currentState].find(currentSymbol) == actionTable[currentState].end()) {
                parseStep.action = "错误：未找到对应动作";
                result.parseSteps.push_back(parseStep);
                result.message = "语法分析失败";
                break;
            }

            std::string action = actionTable[currentState][currentSymbol];
            parseStep.action = action;

            if (action == "acc") {
                result.parseSteps.push_back(parseStep);
                result.success = true;
                result.isAccepted = true;
                result.message = "语法分析成功";
                break;
            } else if (action[0] == 's') {
                // 移入动作
                int nextState = std::stoi(action.substr(1));
                stateStack.push_back(nextState);
                symbolStack.push_back(currentSymbol);
                inputIndex++;
            } else if (action[0] == 'r') {
                // 归约动作
                int productionIndex = std::stoi(action.substr(1));
                const std::string& leftSide = Grammar_SLR1::productionLeftSides[productionIndex];
                const std::vector<std::string>& rightSide = Grammar_SLR1::productionRightSides[productionIndex];

                // 弹出符号和状态
                if (rightSide.size() == 1 && rightSide[0] == "epsilon") {
                    // epsilon产生式，不弹出任何符号
                } else {
                    for (int i = 0; i < rightSide.size(); ++i) {
                        stateStack.pop_back();
                        symbolStack.pop_back();
                    }
                }

                // 压入左部符号
                symbolStack.push_back(leftSide);

                // 查找GOTO状态
                int gotoState = gotoTable[stateStack.back()][leftSide];
                stateStack.push_back(gotoState);
            }

            result.parseSteps.push_back(parseStep);

            // 防止无限循环
            if (step > 1000) {
                result.message = "解析步骤过多，可能存在问题";
                break;
            }
        }

        // 构建分析表
        result.parseTable.headers.push_back("状态");
        for (const std::string& terminal : Grammar_SLR1::terminalSymbols) {
            result.parseTable.headers.push_back(terminal);
        }
        for (const std::string& nonterminal : Grammar_SLR1::nonterminalSymbols) {
            result.parseTable.headers.push_back(nonterminal);
        }

        for (int i = 0; i < actionTable.size(); ++i) {
            ParseTableRow row;
            row.state = i;
            row.actions = actionTable[i];
            row.gotos = gotoTable[i];
            result.parseTable.rows.push_back(row);
        }

        // 构建产生式信息
        for (int i = 0; i < Grammar_SLR1::productionLeftSides.size(); ++i) {
            const std::string& leftSide = Grammar_SLR1::productionLeftSides[i];
            const std::vector<std::string>& rightSide = Grammar_SLR1::productionRightSides[i];
            result.productions[leftSide].push_back(rightSide);
        }

        result.firstSets = firstSets;
        result.followSets = followSets;
        result.dotFile = generateDotFile();

        return result;
    }

    // 生成DOT文件
    std::string generateDotFile() {
        // 转换项目集族为与LR0兼容的格式
        std::vector<std::vector<ItemSet_SLR1::LRItem>> lr0Collection;
        for (const std::set<ItemSet_SLR1::LRItem>& itemSet : canonicalCollection) {
            std::vector<ItemSet_SLR1::LRItem> lr0ItemSet;
            for (const ItemSet_SLR1::LRItem& item : itemSet) {
                ItemSet_SLR1::LRItem lr0Item;
                lr0Item.productionIndex = item.productionIndex;
                lr0Item.dotPosition = item.dotPosition;
                lr0ItemSet.push_back(lr0Item);
            }
            lr0Collection.push_back(lr0ItemSet);
        }

        return generateDotFileContent(lr0Collection, actionTable, gotoTable);
    }

    // 生成DOT文件内容
    std::string generateDotFileContent(const std::vector<std::vector<ItemSet_SLR1::LRItem>>& canonicalCollection,
                                     const std::vector<std::map<std::string, std::string>>& actionTable,
                                     const std::vector<std::map<std::string, int>>& gotoTable) {
        std::ostringstream dot;
        dot << "digraph SLR1_Automaton {\n";
        dot << "    rankdir=LR;\n";
        dot << "    node [shape=box];\n\n";

        // 为每个状态生成节点
        for (int i = 0; i < canonicalCollection.size(); ++i) {
            // 判断是否为接受状态（包含接受项目）
            bool isAcceptState = false;
            for (const ItemSet_SLR1::LRItem& item : canonicalCollection[i]) {
                // 检查是否为增广文法的接受项目 (S' -> S •)
                if (item.productionIndex == 0 && 
                    item.dotPosition == Grammar_SLR1::productionRightSides[item.productionIndex].size()) {
                    isAcceptState = true;
                    break;
                }
            }

            // 设置节点样式
            dot << "    I" << i;
            if (i == 0) {
                // 开始状态样式
                dot << " [style=\"rounded,filled\", fillcolor=lightblue, label=\"I" << i << "\\n";
            } else if (isAcceptState) {
                // 接受状态样式
                dot << " [style=\"rounded,filled\", fillcolor=lightgreen, label=\"I" << i << "\\n";
            } else {
                // 普通状态样式
                dot << " [label=\"I" << i << "\\n";
            }

            // 添加项目
            for (const ItemSet_SLR1::LRItem& item : canonicalCollection[i]) {
                const std::string& leftSide = Grammar_SLR1::productionLeftSides[item.productionIndex];
                const std::vector<std::string>& rightSide = Grammar_SLR1::productionRightSides[item.productionIndex];

                dot << leftSide << " → ";
                for (int j = 0; j < rightSide.size(); ++j) {
                    if (j == item.dotPosition) {
                        dot << "•";
                    }
                    dot << rightSide[j];
                    if (j < rightSide.size() - 1) {
                        dot << " ";
                    }
                }
                if (item.dotPosition == rightSide.size()) {
                    dot << "•";
                }
                dot << "\\n";
            }

            dot << "\"];\n";
        }

        // 生成转换边
        for (int i = 0; i < canonicalCollection.size(); ++i) {
            // ACTION表的移入转换
            for (const auto& action : actionTable[i]) {
                if (action.second[0] == 's') {
                    int targetState = std::stoi(action.second.substr(1));
                    dot << "    I" << i << " -> I" << targetState;
                    dot << " [label=\"" << action.first << "\"];\n";
                }
            }

            // GOTO表的转换
            for (const auto& gotoEntry : gotoTable[i]) {
                dot << "    I" << i << " -> I" << gotoEntry.second;
                dot << " [label=\"" << gotoEntry.first << "\"];\n";
            }
        }

        dot << "}\n";
        return dot.str();
    }

    // 打印语法
    void printGrammar() {
        std::cout << "=== SLR1 语法 ===" << std::endl;
        for (int i = 0; i < Grammar_SLR1::productionLeftSides.size(); ++i) {
            std::cout << i << ": " << Grammar_SLR1::productionLeftSides[i] << " -> ";
            const std::vector<std::string>& rightSide = Grammar_SLR1::productionRightSides[i];
            for (int j = 0; j < rightSide.size(); ++j) {
                std::cout << rightSide[j];
                if (j < rightSide.size() - 1) {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }
    }

    // 打印自动机
    void printAutomaton() {
        std::cout << "=== SLR1 自动机 ===" << std::endl;
        for (int i = 0; i < canonicalCollection.size(); ++i) {
            std::cout << "状态 " << i << ":" << std::endl;
            for (const ItemSet_SLR1::LRItem& item : canonicalCollection[i]) {
                const std::string& leftSide = Grammar_SLR1::productionLeftSides[item.productionIndex];
                const std::vector<std::string>& rightSide = Grammar_SLR1::productionRightSides[item.productionIndex];

                std::cout << "  " << leftSide << " -> ";
                for (int j = 0; j < rightSide.size(); ++j) {
                    if (j == item.dotPosition) {
                        std::cout << "•";
                    }
                    std::cout << rightSide[j];
                    if (j < rightSide.size() - 1) {
                        std::cout << " ";
                    }
                }
                if (item.dotPosition == rightSide.size()) {
                    std::cout << "•";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }

    // 打印FIRST集合
    void printFirstSets() {
        std::cout << "=== FIRST 集合 ===" << std::endl;
        for (const auto& entry : firstSets) {
            std::cout << "FIRST(" << entry.first << ") = { ";
            bool first = true;
            for (const std::string& symbol : entry.second) {
                if (!first) std::cout << ", ";
                std::cout << symbol;
                first = false;
            }
            std::cout << " }" << std::endl;
        }
    }

    // 打印FOLLOW集合
    void printFollowSets() {
        std::cout << "=== FOLLOW 集合 ===" << std::endl;
        for (const auto& entry : followSets) {
            std::cout << "FOLLOW(" << entry.first << ") = { ";
            bool first = true;
            for (const std::string& symbol : entry.second) {
                if (!first) std::cout << ", ";
                std::cout << symbol;
                first = false;
            }
            std::cout << " }" << std::endl;
        }
    }
}
