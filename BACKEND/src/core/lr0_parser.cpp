#include "lr0_parser.h"
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
namespace Grammar {
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

            // 处理大写字母 - 每个大写字母是一个独立的非终结符
            if (std::isupper(currentChar)) {
                // 先保存之前累积的符号
                if (!currentSymbol.empty()) {
                    symbols.push_back(currentSymbol);
                    currentSymbol.clear();
                }
                // 大写字母作为单独的符号
                symbols.push_back(std::string(1, currentChar));
            }
            // 处理小写字母和数字 - 可以组合成终结符
            else if (std::islower(currentChar) || std::isdigit(currentChar)) {
                currentSymbol += currentChar;
            }
            // 处理特殊字符和操作符
            else {
                if (!currentSymbol.empty()) {
                    symbols.push_back(currentSymbol);
                    currentSymbol.clear();
                }
                symbols.push_back(std::string(1, currentChar));
            }
        }

        // 处理最后一个符号
        if (!currentSymbol.empty()) {
            symbols.push_back(currentSymbol);
        }

        return symbols;
    }

    // 从文件中解析语法规则
    bool parseGrammarFromFile(const std::string& filename) {
        std::cout << "Reading grammar file: " << filename << std::endl;
        std::ifstream inputFile(filename);

        if (!inputFile.is_open()) {
            std::cerr << "Error: Cannot open grammar file '" << filename << "'" << std::endl;
            return false;
        }

        std::string currentLine;
        std::vector<std::pair<std::string, std::string>> rawProductionRules;
        std::string startSymbol;
        bool hasStartSymbolS = false;

        // 读取每一行并解析产生式
        int lineNumber = 0;
        while (std::getline(inputFile, currentLine)) {
            lineNumber++;
            currentLine = trimWhitespace(currentLine);

            // 跳过空行和注释 find 返回子串出现的第一个位置index
            if (currentLine.empty() || currentLine.find("//") == 0) continue;

            // 查找箭头符号
            size_t arrowPosition = currentLine.find("->");
            if (arrowPosition == std::string::npos) {
                std::cerr << "Warning: Line " << lineNumber << " does not contain '->'. Skipping." << std::endl;
                continue;
            }

            std::string leftHandSide = trimWhitespace(currentLine.substr(0, arrowPosition));
            std::string rightHandSide = trimWhitespace(currentLine.substr(arrowPosition + 2));

            if (leftHandSide.empty() || rightHandSide.empty()) {
                std::cerr << "Warning: Line " << lineNumber << " has empty left or right side. Skipping." << std::endl;
                continue;
            }

            // 记录第一个产生式的左部作为开始符号 只会执行一次
            if (rawProductionRules.empty()) startSymbol = leftHandSide;
            if (leftHandSide == "S") hasStartSymbolS = true;

            rawProductionRules.emplace_back(leftHandSide, rightHandSide);
        }

        inputFile.close();

        if (rawProductionRules.empty()) {
            std::cerr << "Error: No valid production rules found in the grammar file." << std::endl;
            return false;
        }

        // 总是添加拓广文法的开始符号
        if (!startSymbol.empty()) {
            std::string augmentedStartSymbol = startSymbol + "'";  // 例如：S -> S'
            rawProductionRules.insert(rawProductionRules.begin(), { augmentedStartSymbol, startSymbol });
            std::cout << "Added augmented start production: " << augmentedStartSymbol << " -> " << startSymbol << std::endl;
        }

        // 清空原有数据，重新构建
        productionLeftSides.clear();
        productionRightSides.clear();
        nonterminalSymbols.clear();
        terminalSymbols.clear();
        allSymbols.clear();

        std::set<std::string> nonterminalSet, terminalSet;

        // 处理每个产生式
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

                // 收集终结符和非终结符
                for (const auto& symbol : symbols) {
                    if (isNonterminal(symbol)) {
                        nonterminalSet.insert(symbol);
                    }
                    else {
                        terminalSet.insert(symbol);
                    }
                }

                productionLeftSides.push_back(productionPair.first);
                productionRightSides.push_back(symbols);
            }
        }

        // 构建非终结符和终结符集合
        nonterminalSymbols.assign(nonterminalSet.begin(), nonterminalSet.end());
        terminalSymbols.assign(terminalSet.begin(), terminalSet.end());
        terminalSymbols.push_back("#");  // 添加结束符

        // 构建所有符号集合
        allSymbols = nonterminalSymbols;
        allSymbols.insert(allSymbols.end(), terminalSymbols.begin(), terminalSymbols.end());

        // 输出解析结果
        std::cout << "Grammar parsing completed:" << std::endl;
        std::cout << "  Non-terminals: ";
        for (const auto& nt : nonterminalSymbols) std::cout << nt << " ";
        std::cout << std::endl;
        std::cout << "  Terminals: ";
        for (const auto& t : terminalSymbols) std::cout << t << " ";
        std::cout << std::endl;
        std::cout << "  Total productions: " << productionLeftSides.size() << std::endl;
        std::cout << std::endl;

        return true;
    }

    // API接口函数 - 用于web后端集成
    void readGrammarFromString(const std::string& grammarContent) {
        std::vector<std::pair<std::string, std::string>> rawProductionRules;
        std::string startSymbol;
        bool hasStartSymbolS = false;

        std::istringstream grammarStream(grammarContent);
        std::string currentLine;
        int lineNumber = 0;

        while (std::getline(grammarStream, currentLine)) {
            lineNumber++;
            currentLine = trimWhitespace(currentLine);

            if (currentLine.empty() || currentLine.find("//") == 0) continue;

            size_t arrowPosition = currentLine.find("->");
            if (arrowPosition == std::string::npos) {
                continue;
            }

            std::string leftHandSide = trimWhitespace(currentLine.substr(0, arrowPosition));
            std::string rightHandSide = trimWhitespace(currentLine.substr(arrowPosition + 2));

            if (leftHandSide.empty() || rightHandSide.empty()) {
                continue;
            }

            if (rawProductionRules.empty()) startSymbol = leftHandSide;
            if (leftHandSide == "S") hasStartSymbolS = true;

            rawProductionRules.emplace_back(leftHandSide, rightHandSide);
        }

        if (rawProductionRules.empty()) {
            throw std::runtime_error("No valid production rules found in grammar");
        }

        // 总是添加拓广文法的开始符号
        if (!startSymbol.empty()) {
            std::string augmentedStartSymbol = startSymbol + "'";  // 例如：S -> S'
            rawProductionRules.insert(rawProductionRules.begin(), { augmentedStartSymbol, startSymbol });
        }

        // 清空原有数据，重新构建
        productionLeftSides.clear();
        productionRightSides.clear();
        nonterminalSymbols.clear();
        terminalSymbols.clear();
        allSymbols.clear();

        std::set<std::string> nonterminalSet, terminalSet;

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

            while (std::getline(stringStream, alternativeRule, '|')) {
                alternativeRule = trimWhitespace(alternativeRule);
                if (alternativeRule.empty()) continue;

                std::vector<std::string> symbols = parseRightHandSide(alternativeRule);
                if (symbols.empty()) continue;

                for (const auto& symbol : symbols) {
                    if (isNonterminal(symbol)) {
                        nonterminalSet.insert(symbol);
                    } else {
                        terminalSet.insert(symbol);
                    }
                }

                productionLeftSides.push_back(productionPair.first);
                productionRightSides.push_back(symbols);
            }
        }

        nonterminalSymbols.assign(nonterminalSet.begin(), nonterminalSet.end());
        terminalSymbols.assign(terminalSet.begin(), terminalSet.end());
        terminalSymbols.push_back("#");

        allSymbols = nonterminalSymbols;
        allSymbols.insert(allSymbols.end(), terminalSymbols.begin(), terminalSymbols.end());
    }

    // 获取产生式左部
    std::vector<std::string> getProductionLeftSides() {
        return productionLeftSides;
    }

    // 获取产生式右部
    std::vector<std::vector<std::string>> getProductionRightSides() {
        return productionRightSides;
    }

    // 获取所有非终结符
    std::vector<std::string> getNonterminalSymbols() {
        return nonterminalSymbols;
    }

    // 获取所有终结符
    std::vector<std::string> getTerminalSymbols() {
        return terminalSymbols;
    }

    // API接口函数 - 从文件读取语法
    void readGrammarFromFile(const std::string& filename) {
        if (!parseGrammarFromFile(filename)) {
            throw std::runtime_error("Failed to read grammar from file: " + filename);
        }
    }

    // 打印语法规则
    void printGrammar() {
        std::cout << "Grammar Rules:" << std::endl;
        for (size_t i = 0; i < productionLeftSides.size(); ++i) {
            std::cout << "  " << productionLeftSides[i] << " -> ";
            for (const auto& symbol : productionRightSides[i]) {
                std::cout << symbol << " ";
            }
            std::cout << std::endl;
        }
    }
}

// 项目集和闭包操作命名空间
namespace ItemSet {
    // LR(0)项目结构
    struct LRItem {
        int productionIndex;  // 产生式编号
        int dotPosition;      // 点的位置，表示在dotPosition之前有个点

        // 默认构造函数
        LRItem() : productionIndex(0), dotPosition(0) {}

        // 带参数的构造函数
        LRItem(int prodIndex, int dotPos) : productionIndex(prodIndex), dotPosition(dotPos) {}

        // 比较操作符，用于判断两个项目是否相同
        bool operator==(const LRItem& otherItem) const {
            return (productionIndex == otherItem.productionIndex) && (dotPosition == otherItem.dotPosition);
        }

        // 打印项目
        void printItem() const {
            std::cout << getItemString() << std::endl;
        }

        // 获取项目的字符串表示
        // E -> .A
        std::string getItemString() const {
            std::string itemString = Grammar::productionLeftSides[productionIndex] + " -> ";
            for (int symbolIndex = 0; symbolIndex < Grammar::productionRightSides[productionIndex].size(); symbolIndex++) {
                if (symbolIndex == dotPosition) itemString += ".";
                itemString += Grammar::productionRightSides[productionIndex][symbolIndex] + " ";
            }
            if (dotPosition == Grammar::productionRightSides[productionIndex].size()) itemString += ".";
            return itemString;
        }
    };

    // 计算项目集的闭包
    std::vector<LRItem> computeClosure(std::vector<LRItem> itemSet) {
        size_t totalProductions = Grammar::productionLeftSides.size();
        size_t currentSize = 0;

        // 使用索引而不是标志位，避免重复遍历已处理的项目
        while (currentSize < itemSet.size()) {
            size_t previousSize = itemSet.size();

            for (size_t itemIndex = currentSize; itemIndex < previousSize; ++itemIndex) {
                const LRItem& currentItem = itemSet[itemIndex];

                // 如果点后面还有符号
                if (currentItem.dotPosition < Grammar::productionRightSides[currentItem.productionIndex].size()) {
                    const std::string& symbolAfterDot = Grammar::productionRightSides[currentItem.productionIndex][currentItem.dotPosition];

                    // 如果点后面的符号是非终结符
                    if (Grammar::isNonterminal(symbolAfterDot)) {
                        // 添加该非终结符的所有产生式
                        for (size_t productionIndex = 0; productionIndex < totalProductions; productionIndex++) {
                            if (Grammar::productionLeftSides[productionIndex] == symbolAfterDot) {
                                LRItem newItem(static_cast<int>(productionIndex), 0);

                                // 检查是否已存在
                                bool itemExists = false;
                                for (const auto& existingItem : itemSet) {
                                    if (existingItem == newItem) {
                                        itemExists = true;
                                        break;
                                    }
                                }

                                if (!itemExists) {
                                    itemSet.push_back(newItem);
                                }
                            }
                        }
                    }
                }
            }

            currentSize = previousSize;
        }

        // 排序项目集
        std::sort(itemSet.begin(), itemSet.end(), [](const LRItem& firstItem, const LRItem& secondItem) {
            if (firstItem.productionIndex != secondItem.productionIndex)
                return firstItem.productionIndex < secondItem.productionIndex;
            return firstItem.dotPosition < secondItem.dotPosition;
            });

        return itemSet;
    }

    // 计算GOTO函数
    std::vector<LRItem> computeGoto(const std::vector<LRItem>& sourceItemSet, const std::string& transitionSymbol) {
        std::vector<LRItem> targetItemSet;

        // 对于项目集中的每个项目
        for (const auto& currentItem : sourceItemSet) {
            // 检查点后面是否有符号且符号匹配
            if (currentItem.dotPosition < Grammar::productionRightSides[currentItem.productionIndex].size() &&
                Grammar::productionRightSides[currentItem.productionIndex][currentItem.dotPosition] == transitionSymbol) {
                targetItemSet.emplace_back(currentItem.productionIndex, currentItem.dotPosition + 1);
            }
        }

        // 如果没有转换，返回空集
        if (targetItemSet.empty()) {
            return targetItemSet;
        }

        // 返回新项目集的闭包
        return computeClosure(targetItemSet);
    }
}

// 语法分析器命名空间
namespace Parser {
    // 将分析表保存到文件
    void saveParseTableToFile(const std::vector<std::map<std::string, std::string>>& actionTable,
        const std::vector<std::map<std::string, int>>& gotoTable) {
        std::ofstream outputFile("parse_table.txt");
        if (!outputFile.is_open()) {
            std::cout << "Failed to create parse table file." << std::endl;
            return;
        }

        outputFile << "LR(0) Parse Table" << std::endl;
        outputFile << "------------------------------------------------" << std::endl;

        // 写入表头
        outputFile << std::left << std::setw(7) << "State";
        for (const std::string& terminal : Grammar::terminalSymbols) {
            outputFile << std::left << std::setw(7) << terminal;
        }
        outputFile << "| ";
        for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
            if (nonterminal == "S") continue;
            outputFile << std::left << std::setw(7) << nonterminal;
        }
        outputFile << std::endl;

        // 写入分隔线
        outputFile << "------------------------------------------------" << std::endl;

        // 写入表内容
        for (int stateIndex = 0; stateIndex < actionTable.size(); stateIndex++) {
            outputFile << std::left << std::setw(7) << stateIndex;
            for (const std::string& terminal : Grammar::terminalSymbols) {
                outputFile << std::left << std::setw(7) << actionTable[stateIndex].at(terminal);
            }
            outputFile << "| ";
            for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
                if (nonterminal == "S") continue;
                if (gotoTable[stateIndex].at(nonterminal) != -1)
                    outputFile << std::left << std::setw(7) << gotoTable[stateIndex].at(nonterminal);
                else
                    outputFile << std::left << std::setw(7) << "";
            }
            outputFile << std::endl;
        }
        outputFile.close();
        std::cout << "Parse table saved to parse_table.txt" << std::endl;
    }

    // 解析输入字符串
    void parseInputString(const std::vector<std::map<std::string, std::string>>& actionTable,
        const std::vector<std::map<std::string, int>>& gotoTable,
        const std::string& inputString) {
        // 分词处理输入字符串
        std::vector<std::string> inputTokens;
        std::istringstream iss(inputString);
        std::string token;
        while (iss >> token) {
            inputTokens.push_back(token);
        }
        inputTokens.push_back("#");

        std::vector<int> stateStack;      // 状态栈
        std::vector<std::string> symbolStack; // 符号栈
        stateStack.push_back(0);
        int inputPosition = 0;
        int stepNumber = 1;
        bool isAccepted = false;

        // 存储所有解析步骤的信息，用于计算最优列宽
        struct ParseStep {
            int step;
            std::string stateStack;
            std::string symbolStack;
            std::string remainingInput;
            std::string action;
        };
        std::vector<ParseStep> allSteps;

        // 先进行一遍解析，收集所有信息
        std::vector<int> tempStateStack = stateStack;
        std::vector<std::string> tempSymbolStack = symbolStack;
        int tempInputPosition = inputPosition;
        int tempStepNumber = stepNumber;

        while (true) {
            int currentState = tempStateStack.back();
            
            // 检查输入索引是否越界
            if (tempInputPosition >= inputTokens.size()) {
                ParseStep step;
                step.step = tempStepNumber;
                step.stateStack = "";
                step.symbolStack = "";
                step.remainingInput = "";
                step.action = "error (unexpected end of input)";
                allSteps.push_back(step);
                break;
            }
            
            std::string currentSymbol = inputTokens[tempInputPosition];
            
            // 构建显示字符串
            std::string stateStackString, symbolStackString;
            for (int state : tempStateStack) stateStackString += std::to_string(state) + " ";
            for (auto& symbol : tempSymbolStack) symbolStackString += symbol + " ";
            
            ParseStep step;
            step.step = tempStepNumber;
            step.stateStack = stateStackString;
            step.symbolStack = symbolStackString;
            
            // 构建剩余输入字符串
            std::string remainingInput;
            for (int i = tempInputPosition; i < inputTokens.size(); i++) {
                remainingInput += inputTokens[i];
                if (i < inputTokens.size() - 1) remainingInput += " ";
            }
            step.remainingInput = remainingInput;

            // 检查终结符
            if (std::find(Grammar::terminalSymbols.begin(), Grammar::terminalSymbols.end(), currentSymbol) == Grammar::terminalSymbols.end()) {
                step.action = "Rejected";
                allSteps.push_back(step);
                break;
            }

            auto actionIter = actionTable[currentState].find(currentSymbol);
            if (actionIter == actionTable[currentState].end()) {
                step.action = "error";
                allSteps.push_back(step);
                break;
            }
            
            std::string actionValue = actionIter->second;
            
            if (!actionValue.empty()) {
                if (actionValue[0] == 's') {
                    int nextState = std::stoi(actionValue.substr(1));
                    step.action = "shift " + std::to_string(nextState);
                    tempSymbolStack.push_back(currentSymbol);
                    tempStateStack.push_back(nextState);
                    tempInputPosition++;
                }
                else if (actionValue[0] == 'r') {
                    int productionIndex = std::stoi(actionValue.substr(1));
                    step.action = "reduce " + Grammar::productionLeftSides[productionIndex] + "->";
                    for (auto& symbol : Grammar::productionRightSides[productionIndex]) {
                        step.action += symbol;
                    }
                    int symbolsToReduce = Grammar::productionRightSides[productionIndex].size();
                    for (int j = 0; j < symbolsToReduce; j++) {
                        tempStateStack.pop_back();
                        if (!tempSymbolStack.empty()) tempSymbolStack.pop_back();
                    }
                    int topState = tempStateStack.back();
                    std::string leftSideSymbol = Grammar::productionLeftSides[productionIndex];
                    int gotoState = gotoTable[topState].at(leftSideSymbol);
                    tempSymbolStack.push_back(leftSideSymbol);
                    tempStateStack.push_back(gotoState);
                }
                else if (actionValue == "acc") {
                    step.action = "accept";
                    allSteps.push_back(step);
                    break;
                }
            }
            else {
                step.action = "error";
                allSteps.push_back(step);
                break;
            }
            
            allSteps.push_back(step);
            tempStepNumber++;
        }

        // 计算最优列宽
        const int STEP_WIDTH = 6;
        int maxStateStackWidth = 12;
        int maxSymbolStackWidth = 12;
        int maxInputWidth = 15;
        int maxActionWidth = 12;
        
        for (const auto& step : allSteps) {
            maxStateStackWidth = std::max(maxStateStackWidth, (int)step.stateStack.length());
            maxSymbolStackWidth = std::max(maxSymbolStackWidth, (int)step.symbolStack.length());
            maxInputWidth = std::max(maxInputWidth, (int)step.remainingInput.length());
            maxActionWidth = std::max(maxActionWidth, (int)step.action.length());
        }
        
        // 稍微增加一点padding
        maxStateStackWidth += 2;
        maxSymbolStackWidth += 2;
        maxInputWidth += 2;
        maxActionWidth += 2;

        // 创建输出文件流
        std::ofstream outputFile("parsing_result.txt");
        if (!outputFile.is_open()) {
            std::cerr << "Failed to create parsing result file" << std::endl;
            return;
        }

        std::cout << "Parsing process:" << std::endl;
        outputFile << "Parsing process:" << std::endl;
        
        std::cout << std::left
            << std::setw(STEP_WIDTH) << "Step"
            << std::setw(maxStateStackWidth) << "State_Stack"
            << std::setw(maxSymbolStackWidth) << "Symbol_Stack"
            << std::setw(maxInputWidth) << "Remaining_Input"
            << std::setw(maxActionWidth) << "Action" << std::endl;
        outputFile << std::left
            << std::setw(STEP_WIDTH) << "Step"
            << std::setw(maxStateStackWidth) << "State_Stack"
            << std::setw(maxSymbolStackWidth) << "Symbol_Stack"
            << std::setw(maxInputWidth) << "Remaining_Input"
            << std::setw(maxActionWidth) << "Action" << std::endl;
            
        std::string separator = std::string(STEP_WIDTH + maxStateStackWidth + maxSymbolStackWidth + maxInputWidth + maxActionWidth, '-');
        std::cout << separator << std::endl;
        outputFile << separator << std::endl;

        // 输出所有步骤
        for (const auto& step : allSteps) {
            std::cout << std::left
                << std::setw(STEP_WIDTH) << step.step
                << std::setw(maxStateStackWidth) << step.stateStack
                << std::setw(maxSymbolStackWidth) << step.symbolStack
                << std::setw(maxInputWidth) << step.remainingInput
                << std::setw(maxActionWidth) << step.action << std::endl;

            outputFile << std::left
                << std::setw(STEP_WIDTH) << step.step
                << std::setw(maxStateStackWidth) << step.stateStack
                << std::setw(maxSymbolStackWidth) << step.symbolStack
                << std::setw(maxInputWidth) << step.remainingInput
                << std::setw(maxActionWidth) << step.action << std::endl;
        }


        
        // 判断最终结果
        bool finalAccepted = false;
        for (const auto& step : allSteps) {
            if (step.action == "accept") {
                finalAccepted = true;
                break;
            }
            if (step.action == "Rejected" || step.action == "error") {
                break;
            }
        }
        
        if (finalAccepted) {
            std::cout << "Result: Accepted" << std::endl;
            outputFile << "Result: Accepted" << std::endl;
        }
        else {
            std::cout << "Result: Rejected" << std::endl;
            outputFile << "Result: Rejected" << std::endl;
        }
    }
}

// LR0分析器主逻辑命名空间
namespace LR0Analyzer {
    // 构建所有LR(0)项目集
    std::vector<std::vector<ItemSet::LRItem>> buildAllItemSets() {
        std::vector<std::vector<ItemSet::LRItem>> canonicalCollection;
        
        // 构建初始项目集I0 - 包含拓广开始符号的产生式
        std::vector<ItemSet::LRItem> initialItems;
        
        // 寻找拓广开始符号（以'结尾的符号）或S符号
        int startProductionIndex = -1;
        for (size_t i = 0; i < Grammar::productionLeftSides.size(); i++) {
            const std::string& leftSide = Grammar::productionLeftSides[i];
            if (leftSide == "S" || (leftSide.length() > 1 && leftSide.back() == '\'')) {
                startProductionIndex = static_cast<int>(i);
                break;
            }
        }
        
        // 如果找到了拓广开始符号，用它构建初始项目集
        if (startProductionIndex != -1) {
            initialItems.push_back(ItemSet::LRItem(startProductionIndex, 0));
        } else if (!Grammar::productionLeftSides.empty()) {
            // 如果没有找到，使用第一个产生式
            initialItems.push_back(ItemSet::LRItem(0, 0));
        }
        
        // 添加初始项目集I0
        canonicalCollection.push_back(ItemSet::computeClosure(initialItems));

        // 为每个项目集计算GOTO函数，直到没有新的项目集产生
        for (size_t itemSetIndex = 0; itemSetIndex < canonicalCollection.size(); itemSetIndex++) {
            for (const std::string& transitionSymbol : Grammar::allSymbols) {
                if (transitionSymbol == "S" || transitionSymbol == "#") continue;
                std::vector<ItemSet::LRItem> gotoResult = ItemSet::computeGoto(canonicalCollection[itemSetIndex], transitionSymbol);
                if (!gotoResult.empty()) {
                    bool alreadyExists = false;
                    for (auto& existingItemSet : canonicalCollection) {
                        if (existingItemSet == gotoResult) {
                            alreadyExists = true;
                            break;
                        }
                    }
                    if (!alreadyExists) canonicalCollection.push_back(gotoResult);
                }
            }
        }
        return canonicalCollection;
    }

    // 打印所有项目集
    void printAllItemSets(const std::vector<std::vector<ItemSet::LRItem>>& canonicalCollection) {
        std::cout << "LR(0) Item Sets:" << std::endl;
        std::cout << "=================" << std::endl;
        for (int itemSetIndex = 0; itemSetIndex < canonicalCollection.size(); ++itemSetIndex) {
            std::cout << "I_" << itemSetIndex << ":" << std::endl;
            for (auto currentItem : canonicalCollection[itemSetIndex]) {
                std::cout << "  " << currentItem.getItemString() << std::endl;
            }
            std::cout << std::endl;
        }
    }

    // 初始化分析表
    void initializeParseTables(std::vector<std::map<std::string, std::string>>& actionTable,
        std::vector<std::map<std::string, int>>& gotoTable, int numberOfStates) {
        actionTable.resize(numberOfStates);
        gotoTable.resize(numberOfStates);

        for (int stateIndex = 0; stateIndex < numberOfStates; stateIndex++) {
            for (const std::string& terminal : Grammar::terminalSymbols) {
                actionTable[stateIndex][terminal] = "";
            }
            for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
                gotoTable[stateIndex][nonterminal] = -1;
            }
        }
    }

    // 构建分析表
    void buildParseTables(const std::vector<std::vector<ItemSet::LRItem>>& canonicalCollection,
        std::vector<std::map<std::string, std::string>>& actionTable,
        std::vector<std::map<std::string, int>>& gotoTable) {
        int numberOfStates = canonicalCollection.size();

        std::cout << "Building parse table..." << std::endl;
        std::cout << "=======================" << std::endl;
        for (int stateIndex = 0; stateIndex < numberOfStates; stateIndex++) {
            std::cout << "Processing state " << stateIndex << ":" << std::endl;
            for (auto currentItem : canonicalCollection[stateIndex]) {
                int productionIndex = currentItem.productionIndex;
                int dotPosition = currentItem.dotPosition;
                std::cout << "  Item: " << currentItem.getItemString() << std::endl;

                if (dotPosition < (int)Grammar::productionRightSides[productionIndex].size()) {
                    // 点后面还有符号
                    std::string symbolAfterDot = Grammar::productionRightSides[productionIndex][dotPosition];
                    std::cout << "    Symbol after dot: " << symbolAfterDot << std::endl;

                    // 移入操作（对于终结符）
                    if (std::find(Grammar::terminalSymbols.begin(), Grammar::terminalSymbols.end(), symbolAfterDot) != Grammar::terminalSymbols.end() && symbolAfterDot != "#") {
                        std::vector<ItemSet::LRItem> gotoResult = ItemSet::computeGoto(canonicalCollection[stateIndex], symbolAfterDot);
                        if (!gotoResult.empty()) {
                            int targetStateIndex;
                            for (targetStateIndex = 0; targetStateIndex < numberOfStates; targetStateIndex++) {
                                if (canonicalCollection[targetStateIndex] == gotoResult) {
                                    std::cout << "    Shift to state " << targetStateIndex << std::endl;
                                    break;
                                }
                            }
                            if (targetStateIndex < numberOfStates) {
                                actionTable[stateIndex][symbolAfterDot] = "s" + std::to_string(targetStateIndex);
                            }
                        }
                    }

                    // GOTO操作（对于非终结符）
                    if (std::find(Grammar::nonterminalSymbols.begin(), Grammar::nonterminalSymbols.end(), symbolAfterDot) != Grammar::nonterminalSymbols.end()) {
                        std::vector<ItemSet::LRItem> gotoResult = ItemSet::computeGoto(canonicalCollection[stateIndex], symbolAfterDot);
                        if (!gotoResult.empty()) {
                            int targetStateIndex;
                            for (targetStateIndex = 0; targetStateIndex < canonicalCollection.size(); targetStateIndex++) {
                                if (canonicalCollection[targetStateIndex] == gotoResult) {
                                    std::cout << "    GOTO state " << targetStateIndex << std::endl;
                                    break;
                                }
                            }
                            if (targetStateIndex < numberOfStates) {
                                gotoTable[stateIndex][symbolAfterDot] = targetStateIndex;
                            }
                        }
                    }
                }
                else {
                    // 点在最后，归约或接受操作
                    const std::string& leftSide = Grammar::productionLeftSides[productionIndex];
                    // 检查是否是拓广开始符号（以'结尾）或者是S
                    if (leftSide == "S" || (leftSide.length() > 1 && leftSide.back() == '\'')) {
                        actionTable[stateIndex]["#"] = "acc";
                        std::cout << "    Accept action for " << leftSide << std::endl;
                    }
                    else {
                        std::cout << "    Reduce by production " << productionIndex << ": " << Grammar::productionLeftSides[productionIndex] << " -> ";
                        for (auto& symbol : Grammar::productionRightSides[productionIndex]) std::cout << symbol << " ";
                        std::cout << std::endl;

                        // 对所有终结符设置归约操作
                        for (const std::string& terminal : Grammar::terminalSymbols) {
                            if (terminal != "#" && actionTable[stateIndex][terminal] == "") {
                                actionTable[stateIndex][terminal] = "r" + std::to_string(productionIndex);
                            }
                        }
                        actionTable[stateIndex]["#"] = "r" + std::to_string(productionIndex);
                    }
                }
            }
            std::cout << std::endl;
        }
    }

    // 打印产生式
    void printProductionRules() {
        std::cout << "Productions:" << std::endl;
        std::cout << "============" << std::endl;
        for (int productionIndex = 0; productionIndex < Grammar::productionLeftSides.size(); productionIndex++) {
            std::cout << "(" << productionIndex << ") " << Grammar::productionLeftSides[productionIndex] << " -> ";
            for (auto& symbol : Grammar::productionRightSides[productionIndex]) std::cout << symbol << " ";
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    // 打印分析表
    void printParseTable(const std::vector<std::map<std::string, std::string>>& actionTable,
        const std::vector<std::map<std::string, int>>& gotoTable) {
        std::cout << "LR(0) Parse Table:" << std::endl;
        std::cout << "==================" << std::endl;

        std::cout << std::left << std::setw(7) << "State";
        for (const std::string& terminal : Grammar::terminalSymbols) {
            std::cout << std::left << std::setw(7) << terminal;
        }
        std::cout << "| ";
        for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
            if (nonterminal == "S") continue;
            std::cout << std::left << std::setw(7) << nonterminal;
        }
        std::cout << std::endl;

        // 打印分隔线
        std::cout << "------------------------------------------------------------" << std::endl;

        int numberOfStates = actionTable.size();
        for (int stateIndex = 0; stateIndex < numberOfStates; stateIndex++) {
            std::cout << std::left << std::setw(7) << stateIndex;
            for (const std::string& terminal : Grammar::terminalSymbols) {
                std::cout << std::left << std::setw(7) << actionTable[stateIndex].at(terminal);
            }
            std::cout << "| ";
            for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
                if (nonterminal == "S") continue;
                if (gotoTable[stateIndex].at(nonterminal) != -1)
                    std::cout << std::left << std::setw(7) << gotoTable[stateIndex].at(nonterminal);
                else
                    std::cout << std::left << std::setw(7) << "";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    // 生成 Graphviz DOT 文件用于可视化 DFA
    void generateDotFile(const std::vector<std::vector<ItemSet::LRItem>>& canonicalCollection,
        const std::vector<std::map<std::string, std::string>>& actionTable,
        const std::vector<std::map<std::string, int>>& gotoTable) {
        std::ofstream dotFile("lr0_dfa.dot");
        if (!dotFile.is_open()) {
            std::cout << "Failed to create DOT file." << std::endl;
            return;
        }

        dotFile << "digraph LR0_DFA {" << std::endl;
        dotFile << "    rankdir=LR;" << std::endl;
        dotFile << "    node [shape=box, style=rounded];" << std::endl;
        dotFile << std::endl;

        // 为每个状态生成节点
        for (int stateIndex = 0; stateIndex < canonicalCollection.size(); stateIndex++) {
            dotFile << "    I" << stateIndex << " [label=\"I" << stateIndex << "\\n";

            // 添加项目集内容
            for (const auto& item : canonicalCollection[stateIndex]) {
                dotFile << item.getItemString() << "\\n";
            }
            dotFile << "\"];" << std::endl;
        }

        dotFile << std::endl;

        // 生成状态转换边
        for (int stateIndex = 0; stateIndex < canonicalCollection.size(); stateIndex++) {
            // 处理终结符转换（shift 动作）
            for (const std::string& terminal : Grammar::terminalSymbols) {
                if (terminal == "#") continue;  // 跳过结束符

                auto actionIter = actionTable[stateIndex].find(terminal);
                if (actionIter != actionTable[stateIndex].end()) {
                    std::string action = actionIter->second;
                    if (!action.empty() && action[0] == 's') {
                        int targetState = std::stoi(action.substr(1));
                        dotFile << "    I" << stateIndex << " -> I" << targetState
                            << " [label=\"" << terminal << "\"];" << std::endl;
                    }
                }
            }

            // 处理非终结符转换（GOTO）
            for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
                auto gotoIter = gotoTable[stateIndex].find(nonterminal);
                if (gotoIter != gotoTable[stateIndex].end()) {
                    int targetState = gotoIter->second;
                    if (targetState != -1) {
                        dotFile << "    I" << stateIndex << " -> I" << targetState
                            << " [label=\"" << nonterminal << "\", style=dashed];" << std::endl;
                    }
                }
            }
        }

        // 标记接受状态
        for (int stateIndex = 0; stateIndex < actionTable.size(); stateIndex++) {
            for (const auto& pair : actionTable[stateIndex]) {
                if (pair.second == "acc") {
                    dotFile << "    I" << stateIndex << " [style=\"filled,rounded\", fillcolor=lightgreen];" << std::endl;
                    break;
                }
            }
        }

        dotFile << "}" << std::endl;
        dotFile.close();

        std::cout << "Graphviz DOT file saved to lr0_dfa.dot" << std::endl;
        std::cout << "To generate the visualization, run: dot -Tpng lr0_dfa.dot -o lr0_dfa.png" << std::endl;
        // system("dot -Tpng lr0_dfa.dot -o lr0_dfa.png");
        // system("open lr0_dfa.png");  
        // system("open lr0_dfa.svg");
        system("dot -Tsvg lr0_dfa.dot -o lr0_dfa.svg");
        // system("code lr0_dfa.svg");  
        system("code --new-window lr0_dfa.svg");
    }
}


// LR0Parser 命名空间 - 提供对外API接口
namespace LR0Parser {
    // 生成DOT文件内容的辅助函数
    std::string generateDotFileContent(const std::vector<std::vector<ItemSet::LRItem>>& canonicalCollection,
                                     const std::vector<std::map<std::string, std::string>>& actionTable,
                                     const std::vector<std::map<std::string, int>>& gotoTable) {
        std::stringstream dot;
        
        dot << "digraph LR0_DFA {" << std::endl;
        dot << "    rankdir=LR;" << std::endl;
        dot << "    node [shape=box, style=rounded];" << std::endl;
        dot << std::endl;

        // 为每个状态生成节点
        for (int stateIndex = 0; stateIndex < canonicalCollection.size(); stateIndex++) {
            dot << "    I" << stateIndex << " [label=\"I" << stateIndex << "\\n";

            // 添加项目集内容
            for (const auto& item : canonicalCollection[stateIndex]) {
                dot << item.getItemString() << "\\n";
            }
            dot << "\"];" << std::endl;
        }

        dot << std::endl;

        // 生成状态转换边
        for (int stateIndex = 0; stateIndex < canonicalCollection.size(); stateIndex++) {
            // 处理终结符转换（shift 动作）
            for (const std::string& terminal : Grammar::terminalSymbols) {
                if (terminal == "#") continue;  // 跳过结束符

                auto actionIter = actionTable[stateIndex].find(terminal);
                if (actionIter != actionTable[stateIndex].end()) {
                    std::string action = actionIter->second;
                    if (!action.empty() && action[0] == 's') {
                        int targetState = std::stoi(action.substr(1));
                        dot << "    I" << stateIndex << " -> I" << targetState
                            << " [label=\"" << terminal << "\"];" << std::endl;
                    }
                }
            }

            // 处理非终结符转换（GOTO）
            for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
                auto gotoIter = gotoTable[stateIndex].find(nonterminal);
                if (gotoIter != gotoTable[stateIndex].end()) {
                    int targetState = gotoIter->second;
                    if (targetState != -1) {
                        dot << "    I" << stateIndex << " -> I" << targetState
                            << " [label=\"" << nonterminal << "\", style=dashed];" << std::endl;
                    }
                }
            }
        }

        // 标记接受状态
        for (int stateIndex = 0; stateIndex < actionTable.size(); stateIndex++) {
            for (const auto& pair : actionTable[stateIndex]) {
                if (pair.second == "acc") {
                    dot << "    I" << stateIndex << " [style=\"filled,rounded\", fillcolor=lightgreen];" << std::endl;
                    break;
                }
            }
        }

        // 标记初始状态I0
        dot << "    I0 [style=\"rounded,filled\", fillcolor=lightblue];" << std::endl;

        dot << "}" << std::endl;
        
        return dot.str();
    }

    void readGrammarFromString(const std::string& grammarContent) {
        Grammar::readGrammarFromString(grammarContent);
    }
    
    void readGrammarFromFile(const std::string& filename) {
        Grammar::readGrammarFromFile(filename);
    }
    
    std::vector<std::string> getProductionLeftSides() {
        return Grammar::getProductionLeftSides();
    }
    
    std::vector<std::vector<std::string>> getProductionRightSides() {
        return Grammar::getProductionRightSides();
    }
    
    // 构建解析表（独立于输入分析）
    ParseResult buildParseTable() {
        ParseResult result;
        
        try {
            // 1. 构建LR(0)项目集
            std::vector<std::vector<ItemSet::LRItem>> canonicalCollection = LR0Analyzer::buildAllItemSets();
            int numberOfStates = canonicalCollection.size();
            
            // 2. 初始化分析表
            std::vector<std::map<std::string, std::string>> actionTable;
            std::vector<std::map<std::string, int>> gotoTable;
            LR0Analyzer::initializeParseTables(actionTable, gotoTable, numberOfStates);
            
            // 3. 构建分析表
            LR0Analyzer::buildParseTables(canonicalCollection, actionTable, gotoTable);
            
            // 4. 构建分析表结构
            result.parseTable.headers.clear();
            result.parseTable.headers.push_back("State");
            
            // 添加终结符列
            for (const std::string& terminal : Grammar::terminalSymbols) {
                result.parseTable.headers.push_back(terminal);
            }
            
            // 添加非终结符列（排除拓广开始符号）
            for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
                // 排除拓广开始符号（以'结尾的符号）
                if (nonterminal.length() > 0 && nonterminal.back() != '\'') {
                    result.parseTable.headers.push_back(nonterminal);
                }
            }
            
            // 填充分析表行
            for (int i = 0; i < numberOfStates; i++) {
                ParseTableRow row;
                row.state = i;
                
                // 填充ACTION部分
                for (const std::string& terminal : Grammar::terminalSymbols) {
                    auto it = actionTable[i].find(terminal);
                    if (it != actionTable[i].end() && !it->second.empty()) {
                        row.actions[terminal] = it->second;
                    } else {
                        row.actions[terminal] = "";
                    }
                }
                
                // 填充GOTO部分（排除拓广开始符号）
                for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
                    // 排除拓广开始符号（以'结尾的符号）
                    if (nonterminal.length() > 0 && nonterminal.back() != '\'') {
                        auto it = gotoTable[i].find(nonterminal);
                        if (it != gotoTable[i].end() && it->second != -1) {
                            row.gotos[nonterminal] = it->second;
                        }
                    }
                }
                
                result.parseTable.rows.push_back(row);
            }
            
            // 5. 生成DOT文件内容
            result.dotFile = generateDotFileContent(canonicalCollection, actionTable, gotoTable);
            
            // 6. 获取产生式信息 - 聚合同一左部的所有右部
            auto leftSides = Grammar::getProductionLeftSides();
            auto rightSides = Grammar::getProductionRightSides();
            
            for (size_t i = 0; i < leftSides.size() && i < rightSides.size(); ++i) {
                const std::string& leftSide = leftSides[i];
                const std::vector<std::string>& rightSide = rightSides[i];
                
                // 如果这个左部已经存在，添加到现有列表中；否则创建新列表
                if (result.productions.find(leftSide) == result.productions.end()) {
                    result.productions[leftSide] = std::vector<std::vector<std::string>>();
                }
                result.productions[leftSide].push_back(rightSide);
            }

            // 构建按序号排列的产生式列表
            result.productionList.clear();
            for (size_t i = 0; i < leftSides.size() && i < rightSides.size(); ++i) {
                Production prod;
                prod.index = static_cast<int>(i);
                prod.leftSide = leftSides[i];
                prod.rightSide = rightSides[i];
                result.productionList.push_back(prod);
            }
            
            result.success = true;
            result.message = "解析表构建成功";
            result.isAccepted = false;  // 构建表阶段不涉及输入接受与否
            
        } catch (const std::exception& e) {
            result.success = false;
            result.message = "构建解析表时出错: " + std::string(e.what());
            result.isAccepted = false;
        }
        
        return result;
    }

    // 解析输入字符串（使用已构建的解析表）
    ParseResult parseInput(const std::string& input) {
        ParseResult result;
        
        try {
            // 1. 构建LR(0)项目集
            std::vector<std::vector<ItemSet::LRItem>> canonicalCollection = LR0Analyzer::buildAllItemSets();
            int numberOfStates = canonicalCollection.size();
            
            // 2. 初始化分析表
            std::vector<std::map<std::string, std::string>> actionTable;
            std::vector<std::map<std::string, int>> gotoTable;
            LR0Analyzer::initializeParseTables(actionTable, gotoTable, numberOfStates);
            
            // 3. 构建分析表
            LR0Analyzer::buildParseTables(canonicalCollection, actionTable, gotoTable);
            
            // 4. 先构建分析表结构（确保无论输入分析是否成功都有表可显示）
            result.parseTable.headers.clear();
            result.parseTable.headers.push_back("State");
            
            // 添加终结符列
            for (const std::string& terminal : Grammar::terminalSymbols) {
                result.parseTable.headers.push_back(terminal);
            }
            
            // 添加非终结符列（排除拓广开始符号）
            for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
                // 排除拓广开始符号（以'结尾的符号）
                if (nonterminal.length() > 0 && nonterminal.back() != '\'') {
                    result.parseTable.headers.push_back(nonterminal);
                }
            }
            
            // 填充分析表行
            for (int i = 0; i < numberOfStates; i++) {
                ParseTableRow row;
                row.state = i;
                
                // 填充ACTION部分
                for (const std::string& terminal : Grammar::terminalSymbols) {
                    auto it = actionTable[i].find(terminal);
                    if (it != actionTable[i].end() && !it->second.empty()) {
                        row.actions[terminal] = it->second;
                    } else {
                        row.actions[terminal] = "";
                    }
                }
                
                // 填充GOTO部分（排除拓广开始符号）
                for (const std::string& nonterminal : Grammar::nonterminalSymbols) {
                    // 排除拓广开始符号（以'结尾的符号）
                    if (nonterminal.length() > 0 && nonterminal.back() != '\'') {
                        auto it = gotoTable[i].find(nonterminal);
                        if (it != gotoTable[i].end() && it->second != -1) {
                            row.gotos[nonterminal] = it->second;
                        }
                    }
                }
                
                result.parseTable.rows.push_back(row);
            }
            
            // 5. 生成DOT文件内容
            result.dotFile = generateDotFileContent(canonicalCollection, actionTable, gotoTable);
            
            // 6. 获取产生式信息 - 聚合同一左部的所有右部
            auto leftSides = Grammar::getProductionLeftSides();
            auto rightSides = Grammar::getProductionRightSides();
            
            for (size_t i = 0; i < leftSides.size() && i < rightSides.size(); ++i) {
                const std::string& leftSide = leftSides[i];
                const std::vector<std::string>& rightSide = rightSides[i];
                
                // 如果这个左部已经存在，添加到现有列表中；否则创建新列表
                if (result.productions.find(leftSide) == result.productions.end()) {
                    result.productions[leftSide] = std::vector<std::vector<std::string>>();
                }
                result.productions[leftSide].push_back(rightSide);
            }

            // 构建按序号排列的产生式列表
            result.productionList.clear();
            for (size_t i = 0; i < leftSides.size() && i < rightSides.size(); ++i) {
                Production prod;
                prod.index = static_cast<int>(i);
                prod.leftSide = leftSides[i];
                prod.rightSide = rightSides[i];
                result.productionList.push_back(prod);
            }
            
            // 7. 准备输入字符串 - 分词处理
            std::vector<std::string> inputTokens;
            std::istringstream iss(input);
            std::string token;
            while (iss >> token) {
                inputTokens.push_back(token);
            }
            inputTokens.push_back("#");
            
            // 8. 执行解析过程
            std::vector<int> stateStack;
            std::vector<std::string> symbolStack;
            stateStack.push_back(0);
            int inputPosition = 0;
            int stepNumber = 1;
            bool isAccepted = false;
            
            // 存储解析步骤
            std::vector<ParseStep> parseSteps;
            
            while (true) {
                int currentState = stateStack.back();
                
                // 检查输入索引是否越界
                if (inputPosition >= inputTokens.size()) {
                    ParseStep step;
                    step.step = stepNumber;
                    std::string stateStackString, symbolStackString;
                    for (int state : stateStack) {
                        stateStackString += std::to_string(state) + " ";
                    }
                    for (const auto& symbol : symbolStack) {
                        symbolStackString += symbol + " ";
                    }
                    step.stateStack = stateStackString;
                    step.symbolStack = symbolStackString;
                    step.remainingInput = "";
                    step.action = "error (unexpected end of input)";
                    parseSteps.push_back(step);
                    result.isAccepted = false;
                    break;
                }
                
                std::string currentSymbol = inputTokens[inputPosition];
                
                // 构建状态栈和符号栈字符串
                std::string stateStackString, symbolStackString;
                for (int state : stateStack) {
                    stateStackString += std::to_string(state) + " ";
                }
                for (const auto& symbol : symbolStack) {
                    symbolStackString += symbol + " ";
                }
                
                ParseStep step;
                step.step = stepNumber;
                step.stateStack = stateStackString;
                step.symbolStack = symbolStackString;
                
                // 构建剩余输入字符串
                std::string remainingInput;
                for (int i = inputPosition; i < inputTokens.size(); i++) {
                    remainingInput += inputTokens[i];
                    if (i < inputTokens.size() - 1) remainingInput += " ";
                }
                step.remainingInput = remainingInput;
                
                // 检查终结符是否合法
                if (std::find(Grammar::terminalSymbols.begin(), Grammar::terminalSymbols.end(), currentSymbol) == Grammar::terminalSymbols.end()) {
                    step.action = "error (invalid symbol)";
                    parseSteps.push_back(step);
                    result.isAccepted = false;
                    break;
                }
                
                auto actionIter = actionTable[currentState].find(currentSymbol);
                if (actionIter == actionTable[currentState].end() || actionIter->second.empty()) {
                    step.action = "error (no action)";
                    parseSteps.push_back(step);
                    result.isAccepted = false;
                    break;
                }
                
                std::string actionValue = actionIter->second;
                
                if (actionValue[0] == 's') {  // Shift action
                    int nextState = std::stoi(actionValue.substr(1));
                    step.action = "shift " + std::to_string(nextState);
                    parseSteps.push_back(step);
                    
                    stateStack.push_back(nextState);
                    symbolStack.push_back(currentSymbol);
                    inputPosition++;
                    
                } else if (actionValue[0] == 'r') {  // Reduce action
                    int productionIndex = std::stoi(actionValue.substr(1));
                    step.action = "reduce " + std::to_string(productionIndex);
                    parseSteps.push_back(step);
                    
                    // 获取产生式
                    std::string leftSide = Grammar::productionLeftSides[productionIndex];
                    std::vector<std::string> rightSide = Grammar::productionRightSides[productionIndex];
                    
                    // 弹出栈
                    for (int i = 0; i < rightSide.size(); i++) {
                        if (!stateStack.empty()) stateStack.pop_back();
                        if (!symbolStack.empty()) symbolStack.pop_back();
                    }
                    
                    // GOTO操作
                    if (!stateStack.empty()) {
                        int currentState = stateStack.back();
                        int gotoState = gotoTable[currentState][leftSide];
                        if (gotoState != -1) {
                            stateStack.push_back(gotoState);
                            symbolStack.push_back(leftSide);
                        }
                    }
                    
                } else if (actionValue == "acc") {  // Accept
                    step.action = "accept";
                    parseSteps.push_back(step);
                    result.isAccepted = true;
                    isAccepted = true;
                    break;
                } else {
                    step.action = "error (unknown action)";
                    parseSteps.push_back(step);
                    result.isAccepted = false;
                    break;
                }
                
                stepNumber++;
            }
            
            // 9. 设置结果
            result.parseSteps = parseSteps;
            result.success = true;  // 总是设置为成功，因为解析表已构建
            result.message = isAccepted ? "Input accepted" : "Input rejected";
            
        } catch (const std::exception& e) {
            result.success = false;
            result.message = "Error during parsing: " + std::string(e.what());
            result.isAccepted = false;
        }
        
        return result;
    }
    
    void printGrammar() {
        Grammar::printGrammar();
    }
    
    void printAutomaton() {
        // 自动机打印功能待实现
        std::cout << "Automaton printing not yet implemented" << std::endl;
    }
    
    std::string generateDotFile() {
        try {
            // 1. 构建LR(0)项目集
            std::vector<std::vector<ItemSet::LRItem>> canonicalCollection = LR0Analyzer::buildAllItemSets();
            int numberOfStates = canonicalCollection.size();
            
            // 2. 初始化分析表
            std::vector<std::map<std::string, std::string>> actionTable;
            std::vector<std::map<std::string, int>> gotoTable;
            LR0Analyzer::initializeParseTables(actionTable, gotoTable, numberOfStates);
            
            // 3. 构建分析表
            LR0Analyzer::buildParseTables(canonicalCollection, actionTable, gotoTable);
            
            // 4. 生成DOT内容
            return generateDotFileContent(canonicalCollection, actionTable, gotoTable);
            
        } catch (const std::exception& e) {
            return "// Error generating DOT file: " + std::string(e.what());
        }
    }
}


/*
int main() {
    // 1. 解析语法文件
    if (!Grammar::parseGrammarFromFile("grammar.txt")) {
        std::cerr << "Failed to parse grammar file. Exiting." << std::endl;
        return 0;
    }

    // 2. 构建LR(0)项目集
    std::vector<std::vector<ItemSet::LRItem>> canonicalCollection = LR0Analyzer::buildAllItemSets();

    // 3. 打印项目集
    LR0Analyzer::printAllItemSets(canonicalCollection);

    int numberOfStates = canonicalCollection.size();

    // 4. 初始化分析表
    std::vector<std::map<std::string, std::string>> actionTable;
    std::vector<std::map<std::string, int>> gotoTable;
    LR0Analyzer::initializeParseTables(actionTable, gotoTable, numberOfStates);

    // 5. 构建分析表
    LR0Analyzer::buildParseTables(canonicalCollection, actionTable, gotoTable);

    // 6. 打印产生式
    LR0Analyzer::printProductionRules();

    // 7. 打印分析表
    LR0Analyzer::printParseTable(actionTable, gotoTable);

    // 8. 保存分析表到文件
    Parser::saveParseTableToFile(actionTable, gotoTable);

    // 9. 生成 Graphviz DOT 文件
    LR0Analyzer::generateDotFile(canonicalCollection, actionTable, gotoTable);

    // 10. 解析输入字符串
    std::string inputString;
    std::cout << "Enter input string to parse: ";

    // 清除可能存在的换行符，但只在需要时
    if (std::cin.peek() == '\n') {
        std::cin.ignore();
    }

    std::getline(std::cin, inputString);

    // 去除输入字符串中的所有空格
    inputString.erase(std::remove_if(inputString.begin(), inputString.end(),
        [](char c) { return std::isspace(c); }), inputString.end());

    std::cout << "Input string after processing: '" << inputString << "'" << std::endl;
    Parser::parseInputString(actionTable, gotoTable, inputString);
    return 0;
}
*/