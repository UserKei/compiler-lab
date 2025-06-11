#include "crow.h"
#include "api/handlers.h"
#include "api/routes.h"
#include <iostream>
#include <string>

int main() {
    // 创建Crow应用
    crow::SimpleApp app;
    
    // 设置服务器超时和性能参数
    app.server_name("Compiler Tools API Server")
       .timeout(30)         // 30秒请求超时
       .concurrency(4);     // 并发线程数
    
    // 全局CORS中间件
    CROW_ROUTE(app, "/api/<string>").methods("OPTIONS"_method)
    ([](const crow::request& req, const std::string& path) {
        crow::response res(200);
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
        res.add_header("Access-Control-Max-Age", "86400");
        return res;
    });
    
    CROW_ROUTE(app, "/api/<string>/<string>").methods("OPTIONS"_method)
    ([](const crow::request& req, const std::string& path1, const std::string& path2) {
        crow::response res(200);
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
        res.add_header("Access-Control-Max-Age", "86400");
        return res;
    });
    
    // 设置API路由
    APIRoutes::setupRoutes(app);
    
    // 提供静态文件服务（可选）
    CROW_ROUTE(app, "/")
    ([](const crow::request& req) {
        return R"(
<!DOCTYPE html>
<html>
<head>
    <title>Compiler Tools API</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }
        .container { max-width: 800px; margin: 0 auto; }
        .endpoint { background: #f5f5f5; padding: 15px; margin: 10px 0; border-radius: 5px; }
        .method { background: #007bff; color: white; padding: 5px 10px; border-radius: 3px; font-weight: bold; }
        .method.POST { background: #28a745; }
        .method.GET { background: #17a2b8; }
        h1 { color: #333; }
        h2 { color: #666; }
        pre { background: #f8f9fa; padding: 10px; border-radius: 3px; overflow-x: auto; }
        .status { background: #d4edda; color: #155724; padding: 10px; border-radius: 5px; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Compiler Tools API</h1>
        <div class="status">
            <strong>Status:</strong> Server is running on http://localhost:8080
        </div>
        <p>Welcome to the Compiler Tools API - featuring LR0 syntax analyzer and regex automata builder!</p>
        
        <h2>Available Endpoints:</h2>
        
        <div class="endpoint">
            <span class="method GET">GET</span> <strong>/api/health</strong>
            <p>Health check endpoint</p>
        </div>
        
        <div class="endpoint">
            <span class="method POST">POST</span> <strong>/api/lr0/parse</strong>
            <p>Parse input using LR0 syntax analyzer</p>
            <pre>
{
  "grammar": "E -> E + T | T\\nT -> (E) | a",
  "input": "a+a"
}
            </pre>
        </div>
        
        <div class="endpoint">
            <span class="method POST">POST</span> <strong>/api/regex/build</strong>
            <p>Build NFA and DFA from regular expression</p>
            <pre>
{
  "regex": "a*b+"
}
            </pre>
        </div>
        
        <div class="endpoint">
            <span class="method POST">POST</span> <strong>/api/regex/match</strong>
            <p>Test if input string matches regular expression</p>
            <pre>
{
  "regex": "a*b+",
  "input": "aaabbb"
}
            </pre>
        </div>
        
        <div class="endpoint">
            <span class="method POST">POST</span> <strong>/api/grammar/upload</strong>
            <p>Upload grammar file</p>
            <pre>
{
  "content": "E -> E + T | T\\nT -> (E) | a",
  "filename": "my_grammar.txt"
}
            </pre>
        </div>
        
        <h2>Usage Examples:</h2>
        <pre>
# Health check
curl -X GET http://localhost:8080/api/health

# LR0 parsing
curl -X POST http://localhost:8080/api/lr0/parse \\
  -H "Content-Type: application/json" \\
  -d '{"grammar": "E -> E + T | T\\nT -> (E) | a", "input": "a+a"}'

# Regex matching
curl -X POST http://localhost:8080/api/regex/match \\
  -H "Content-Type: application/json" \\
  -d '{"regex": "a*b+", "input": "aaabbb"}'
        </pre>
    </div>
</body>
</html>
        )";
    });
    
    // 设置端口和启动信息
    const int port = 8080;
    
    std::cout << "Starting Compiler Tools API server...\n";
    std::cout << "Server running on: http://localhost:" << port << "\n";
    std::cout << "Available endpoints:\n";
    std::cout << "   GET  /api/health - Health check\n";
    std::cout << "   POST /api/lr0/parse - LR0 syntax analysis\n";
    std::cout << "   POST /api/slr1/parse - SLR1 syntax analysis\n";
    std::cout << "   POST /api/regex/build - Build regex automata\n";
    std::cout << "   POST /api/regex/match - Match string with regex\n";
    std::cout << "   POST /api/grammar/upload - Upload grammar file\n";
    std::cout << "Documentation: http://localhost:" << port << "\n";
    std::cout << "Press Ctrl+C to stop the server\n\n";
    
    // 启动服务器
    app.port(port).multithreaded().run();
    
    return 0;
}
