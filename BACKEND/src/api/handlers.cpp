#include "handlers.h"
#include "../core/lr0_parser.h"
#include "../core/regex_automata.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>
#include <cstdlib>

namespace APIHandlers {
    
    // DOT转SVG的辅助函数
    std::string generateSVGFromDot(const std::string& dotContent) {
        // 创建临时DOT文件
        std::string tempDotFile = "/tmp/lr0_temp.dot";
        std::string tempSvgFile = "/tmp/lr0_temp.svg";
        
        // 写入DOT内容
        std::ofstream dotFile(tempDotFile);
        dotFile << dotContent;
        dotFile.close();
        
        // 使用dot命令生成SVG
        std::string command = "dot -Tsvg " + tempDotFile + " -o " + tempSvgFile;
        int result = system(command.c_str());
        
        if (result == 0) {
            // 读取生成的SVG内容
            std::ifstream svgFile(tempSvgFile);
            std::string svgContent((std::istreambuf_iterator<char>(svgFile)),
                                   std::istreambuf_iterator<char>());
            
            // 清理临时文件
            std::remove(tempDotFile.c_str());
            std::remove(tempSvgFile.c_str());
            
            return svgContent;
        } else {
            return ""; // DOT命令执行失败
        }
    }
    
    // 健康检查端点
    crow::response handleHealthCheck(const crow::request& req) {
        try {
            crow::json::wvalue response;
            response["status"] = "healthy";
            response["message"] = "Compiler Tools API is running";
            response["timestamp"] = (int64_t)std::time(nullptr);
            
            crow::response res(200, response);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Content-Type", "application/json");
            return res;
        } catch (const std::exception& e) {
            crow::json::wvalue error;
            error["error"] = "Health check failed";
            crow::response res(500, error);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
    }
    
    // LR0语法分析端点
    crow::response handleLR0Parse(const crow::request& req) {
        auto startTime = std::chrono::steady_clock::now();
        const int MAX_PROCESSING_TIME_SECONDS = 20; // 最大处理时间20秒
        
        try {
            auto jsonBody = crow::json::load(req.body);
            if (!jsonBody) {
                crow::json::wvalue error;
                error["error"] = "Invalid JSON format";
                return crow::response(400, error);
            }
            
            // 获取请求参数
            std::string grammar = jsonBody["grammar"].s();
            std::string input = jsonBody["input"].s();
            
            if (grammar.empty()) {
                crow::json::wvalue error;
                error["error"] = "Grammar is required";
                return crow::response(400, error);
            }
            
            if (input.empty()) {
                crow::json::wvalue error;
                error["error"] = "Input string is required";
                return crow::response(400, error);
            }
            
            // 读取语法
            LR0Parser::readGrammarFromString(grammar);
            
            // 执行LR0解析
            auto result = LR0Parser::parseInput(input);
            
            // 构建响应
            crow::json::wvalue response;
            response["success"] = result.success;
            response["message"] = result.message;
            response["isAccepted"] = result.isAccepted;
            
            // 解析步骤
            crow::json::wvalue parseSteps(crow::json::type::List);
            for (size_t i = 0; i < result.parseSteps.size(); ++i) {
                const auto& step = result.parseSteps[i];
                crow::json::wvalue stepJson;
                stepJson["step"] = step.step;
                stepJson["stateStack"] = step.stateStack;
                stepJson["symbolStack"] = step.symbolStack;
                stepJson["remainingInput"] = step.remainingInput;
                stepJson["action"] = step.action;
                parseSteps[i] = std::move(stepJson);
            }
            response["parseSteps"] = std::move(parseSteps);
            
            // 分析表
            crow::json::wvalue parseTable;
            crow::json::wvalue headers(crow::json::type::List);
            for (size_t i = 0; i < result.parseTable.headers.size(); ++i) {
                headers[i] = result.parseTable.headers[i];
            }
            parseTable["headers"] = std::move(headers);
            
            crow::json::wvalue rows(crow::json::type::List);
            for (size_t i = 0; i < result.parseTable.rows.size(); ++i) {
                const auto& row = result.parseTable.rows[i];
                crow::json::wvalue rowJson;
                rowJson["state"] = row.state;
                
                // Actions
                crow::json::wvalue actions;
                for (const auto& action : row.actions) {
                    actions[action.first] = action.second;
                }
                rowJson["actions"] = std::move(actions);
                
                // Gotos
                crow::json::wvalue gotos;
                for (const auto& gotoItem : row.gotos) {
                    gotos[gotoItem.first] = gotoItem.second;
                }
                rowJson["gotos"] = std::move(gotos);
                
                rows[i] = std::move(rowJson);
            }
            parseTable["rows"] = std::move(rows);
            response["parseTable"] = std::move(parseTable);
            
            // 生成SVG图表
            if (!result.dotFile.empty()) {
                std::string svgContent = generateSVGFromDot(result.dotFile);
                response["svgDiagram"] = svgContent;
            } else {
                response["svgDiagram"] = "";
            }
            
            // 产生式
            crow::json::wvalue productions;
            for (const auto& prod : result.productions) {
                crow::json::wvalue rightSides(crow::json::type::List);
                for (size_t i = 0; i < prod.second.size(); ++i) {
                    rightSides[i] = prod.second[i];
                }
                productions[prod.first] = std::move(rightSides);
            }
            response["productions"] = std::move(productions);
            
            auto endTime = std::chrono::steady_clock::now();
            auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            std::cout << "[INFO] LR0 parse request processed in " << totalDuration.count() << "ms" << std::endl;
            
            crow::response res(200, response);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Content-Type", "application/json");
            return res;
            
        } catch (const std::exception& e) {
            crow::json::wvalue error;
            error["error"] = "Internal server error: " + std::string(e.what());
            crow::response res(500, error);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
    }
    
    // 正则表达式自动机构建端点
    crow::response handleRegexBuild(const crow::request& req) {
        auto startTime = std::chrono::steady_clock::now();
        const int MAX_PROCESSING_TIME_SECONDS = 10; // 最大处理时间10秒
        
        try {
            auto jsonBody = crow::json::load(req.body);
            if (!jsonBody) {
                crow::json::wvalue error;
                error["error"] = "Invalid JSON format";
                crow::response res(400, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            std::string regex = jsonBody["regex"].s();
            
            if (regex.empty()) {
                crow::json::wvalue error;
                error["error"] = "Regex is required";
                crow::response res(400, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            // 检查正则表达式长度，防止过于复杂的表达式
            if (regex.length() > 1000) {
                crow::json::wvalue error;
                error["error"] = "Regex too long (max 1000 characters)";
                crow::response res(400, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            RegexResult result;
            
            // 在一个简单的超时检查中执行构建
            auto buildStart = std::chrono::steady_clock::now();
            result.success = RegexAutomata::buildFromRegex(regex);
            auto buildEnd = std::chrono::steady_clock::now();
            
            auto buildDuration = std::chrono::duration_cast<std::chrono::seconds>(buildEnd - buildStart);
            if (buildDuration.count() > MAX_PROCESSING_TIME_SECONDS) {
                crow::json::wvalue error;
                error["error"] = "Regex processing timeout (max 10 seconds)";
                crow::response res(408, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            if (result.success) {
                result.message = "Automata built successfully";
                result.nfaDescription = RegexAutomata::getNFADescription();
                result.dfaDescription = RegexAutomata::getDFADescription();
            } else {
                result.message = "Failed to build automata";
            }
            
            auto endTime = std::chrono::steady_clock::now();
            auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            std::cout << "[INFO] Regex build request processed in " << totalDuration.count() << "ms" << std::endl;
            
            crow::response res(200, resultToJson(result));
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Content-Type", "application/json");
            return res;
            
        } catch (const std::exception& e) {
            crow::json::wvalue error;
            error["error"] = "Internal server error: " + std::string(e.what());
            crow::response res(500, error);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
    }
    
    // 正则表达式匹配端点
    crow::response handleRegexMatch(const crow::request& req) {
        auto startTime = std::chrono::steady_clock::now();
        const int MAX_PROCESSING_TIME_SECONDS = 15; // 最大处理时间15秒
        
        try {
            auto jsonBody = crow::json::load(req.body);
            if (!jsonBody) {
                crow::json::wvalue error;
                error["error"] = "Invalid JSON format";
                crow::response res(400, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            std::string regex = jsonBody["regex"].s();
            std::string input = jsonBody["input"].s();
            
            if (regex.empty() || input.empty()) {
                crow::json::wvalue error;
                error["error"] = "Both regex and input are required";
                crow::response res(400, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            // 检查输入长度限制
            if (regex.length() > 1000 || input.length() > 10000) {
                crow::json::wvalue error;
                error["error"] = "Input too long (regex max 1000 chars, input max 10000 chars)";
                crow::response res(400, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            RegexResult result;
            
            // 首先构建自动机（带超时检查）
            auto buildStart = std::chrono::steady_clock::now();
            if (RegexAutomata::buildFromRegex(regex)) {
                auto buildEnd = std::chrono::steady_clock::now();
                auto buildDuration = std::chrono::duration_cast<std::chrono::seconds>(buildEnd - buildStart);
                
                if (buildDuration.count() > MAX_PROCESSING_TIME_SECONDS / 2) {
                    crow::json::wvalue error;
                    error["error"] = "Regex build timeout";
                    crow::response res(408, error);
                    res.add_header("Access-Control-Allow-Origin", "*");
                    return res;
                }
                
                result.success = true;
                
                // 执行匹配（带超时检查）
                auto matchStart = std::chrono::steady_clock::now();
                result.matchResult = RegexAutomata::matchString(input);
                auto matchEnd = std::chrono::steady_clock::now();
                
                auto matchDuration = std::chrono::duration_cast<std::chrono::seconds>(matchEnd - matchStart);
                if (matchDuration.count() > MAX_PROCESSING_TIME_SECONDS / 2) {
                    crow::json::wvalue error;
                    error["error"] = "Regex match timeout";
                    crow::response res(408, error);
                    res.add_header("Access-Control-Allow-Origin", "*");
                    return res;
                }
                
                result.message = result.matchResult ? "String matches regex" : "String does not match regex";
                result.nfaDescription = RegexAutomata::getNFADescription();
                result.dfaDescription = RegexAutomata::getDFADescription();
            } else {
                result.success = false;
                result.message = "Failed to build automata from regex";
                result.matchResult = false;
            }
            
            auto endTime = std::chrono::steady_clock::now();
            auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            std::cout << "[INFO] Regex match request processed in " << totalDuration.count() << "ms" << std::endl;
            
            crow::response res(200, resultToJson(result));
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Content-Type", "application/json");
            return res;
            
        } catch (const std::exception& e) {
            crow::json::wvalue error;
            error["error"] = "Internal server error: " + std::string(e.what());
            crow::response res(500, error);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
    }
    
    // 语法文件上传端点
    crow::response handleGrammarUpload(const crow::request& req) {
        try {
            auto jsonBody = crow::json::load(req.body);
            if (!jsonBody) {
                crow::json::wvalue error;
                error["error"] = "Invalid JSON format";
                crow::response res(400, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            std::string grammarContent = jsonBody["content"].s();
            std::string filename = "uploaded_grammar.txt";
            if (jsonBody.has("filename")) {
                filename = jsonBody["filename"].s();
            }
            
            if (grammarContent.empty()) {
                crow::json::wvalue error;
                error["error"] = "Grammar content is required";
                crow::response res(400, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            // 保存到文件
            std::ofstream file(filename);
            if (file.is_open()) {
                file << grammarContent;
                file.close();
                
                crow::json::wvalue response;
                response["success"] = true;
                response["message"] = "Grammar file uploaded successfully";
                response["filename"] = filename;
                
                crow::response res(200, response);
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;
            } else {
                crow::json::wvalue error;
                error["error"] = "Failed to save grammar file";
                crow::response res(500, error);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
        } catch (const std::exception& e) {
            crow::json::wvalue error;
            error["error"] = "Internal server error: " + std::string(e.what());
            crow::response res(500, error);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
    }
    
    // 辅助函数实现
    std::string vectorToJson(const std::vector<std::string>& vec) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            oss << "\"" << vec[i] << "\"";
            if (i < vec.size() - 1) oss << ",";
        }
        oss << "]";
        return oss.str();
    }
    
    std::string mapToJson(const std::map<std::string, std::vector<std::string>>& map) {
        std::ostringstream oss;
        oss << "{";
        auto it = map.begin();
        for (; it != map.end(); ++it) {
            oss << "\"" << it->first << "\":" << vectorToJson(it->second);
            if (std::next(it) != map.end()) oss << ",";
        }
        oss << "}";
        return oss.str();
    }
    
    crow::json::wvalue resultToJson(const LR0ParseResult& result) {
        crow::json::wvalue json;
        json["success"] = result.success;
        json["message"] = result.message;
        
        // 添加解析步骤
        json["parseSteps"] = crow::json::wvalue::list();
        for (size_t i = 0; i < result.parseSteps.size(); ++i) {
            json["parseSteps"][i] = result.parseSteps[i];
        }
        
        // 添加产生式
        json["productions"] = crow::json::wvalue::object();
        for (const auto& prod : result.productions) {
            json["productions"][prod.first] = crow::json::wvalue::list();
            for (size_t i = 0; i < prod.second.size(); ++i) {
                json["productions"][prod.first][i] = prod.second[i];
            }
        }
        
        return json;
    }
    
    crow::json::wvalue resultToJson(const RegexResult& result) {
        crow::json::wvalue json;
        json["success"] = result.success;
        json["message"] = result.message;
        json["nfaDescription"] = result.nfaDescription;
        json["dfaDescription"] = result.dfaDescription;
        json["matchResult"] = result.matchResult;
        
        return json;
    }
}
