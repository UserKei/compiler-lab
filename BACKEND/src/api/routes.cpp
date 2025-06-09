#include "crow.h"
#include "handlers.h"

namespace APIRoutes {
    void setupRoutes(crow::SimpleApp& app) {
        // 健康检查端点
        CROW_ROUTE(app, "/api/health").methods("GET"_method)
        ([](const crow::request& req) {
            return APIHandlers::handleHealthCheck(req);
        });
        
        CROW_ROUTE(app, "/api/health").methods("OPTIONS"_method)
        ([](const crow::request& req) {
            crow::response res(200);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
            res.add_header("Access-Control-Max-Age", "86400");
            return res;
        });
        
        // LR0语法分析端点
        CROW_ROUTE(app, "/api/lr0/parse").methods("POST"_method)
        ([](const crow::request& req) {
            return APIHandlers::handleLR0Parse(req);
        });
        
        CROW_ROUTE(app, "/api/lr0/parse").methods("OPTIONS"_method)
        ([](const crow::request& req) {
            crow::response res(200);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
            res.add_header("Access-Control-Max-Age", "86400");
            return res;
        });
        
        // SLR1语法分析端点
        CROW_ROUTE(app, "/api/slr1/parse").methods("POST"_method)
        ([](const crow::request& req) {
            return APIHandlers::handleSLR1Parse(req);
        });
        
        CROW_ROUTE(app, "/api/slr1/parse").methods("OPTIONS"_method)
        ([](const crow::request& req) {
            crow::response res(200);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
            res.add_header("Access-Control-Max-Age", "86400");
            return res;
        });
        
        // 正则表达式构建端点
        CROW_ROUTE(app, "/api/regex/build").methods("POST"_method)
        ([](const crow::request& req) {
            return APIHandlers::handleRegexBuild(req);
        });
        
        CROW_ROUTE(app, "/api/regex/build").methods("OPTIONS"_method)
        ([](const crow::request& req) {
            crow::response res(200);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
            res.add_header("Access-Control-Max-Age", "86400");
            return res;
        });
        
        // 正则表达式匹配端点
        CROW_ROUTE(app, "/api/regex/match").methods("POST"_method)
        ([](const crow::request& req) {
            return APIHandlers::handleRegexMatch(req);
        });
        
        CROW_ROUTE(app, "/api/regex/match").methods("OPTIONS"_method)
        ([](const crow::request& req) {
            crow::response res(200);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
            res.add_header("Access-Control-Max-Age", "86400");
            return res;
        });
        
        // 语法文件上传端点
        CROW_ROUTE(app, "/api/grammar/upload").methods("POST"_method)
        ([](const crow::request& req) {
            return APIHandlers::handleGrammarUpload(req);
        });
        
        CROW_ROUTE(app, "/api/grammar/upload").methods("OPTIONS"_method)
        ([](const crow::request& req) {
            crow::response res(200);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
            res.add_header("Access-Control-Max-Age", "86400");
            return res;
        });
    }
}
