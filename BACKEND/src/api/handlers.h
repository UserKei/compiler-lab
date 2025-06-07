#pragma once
#include <string>
#include <vector>
#include <map>
#include "crow.h"

// API处理器命名空间
namespace APIHandlers {
    
    // LR0语法分析相关
    struct LR0ParseResult {
        bool success;
        std::string message;
        std::vector<std::string> parseSteps;
        std::map<std::string, std::vector<std::string>> productions;
    };
    
    // 正则表达式自动机相关
    struct RegexResult {
        bool success;
        std::string message;
        std::string nfaDescription;
        std::string dfaDescription;
        bool matchResult;
        std::string nfaSvg;
        std::string dfaSvg;
        std::string minimizedDfaSvg;
    };
    
    // API端点处理函数
    crow::response handleLR0Parse(const crow::request& req);
    crow::response handleRegexBuild(const crow::request& req);
    crow::response handleRegexMatch(const crow::request& req);
    crow::response handleGrammarUpload(const crow::request& req);
    crow::response handleHealthCheck(const crow::request& req);
    
    // 辅助函数
    std::string vectorToJson(const std::vector<std::string>& vec);
    std::string mapToJson(const std::map<std::string, std::vector<std::string>>& map);
    crow::json::wvalue resultToJson(const LR0ParseResult& result);
    crow::json::wvalue resultToJson(const RegexResult& result);
}
