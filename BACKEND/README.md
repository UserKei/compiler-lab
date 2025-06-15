# 编译器工具后端 API

基于 C++ Crow 框架构建的 RESTful Web API，提供 LR0 语法分析器和正则表达式自动机构建工具的访问接口。

## 项目结构

```
compiler-tools-backend/
├── src/
│   ├── main.cpp               # 应用程序入口点
│   ├── core/                  # 核心编译器功能
│   │   ├── lr0_parser.h       # LR0 解析器接口
│   │   ├── lr0_parser.cpp     # LR0 语法分析器实现
│   │   ├── regex_automata.h   # 正则表达式自动机接口
│   │   └── regex_automata.cpp # 正则表达式自动机构建器实现
│   └── api/                   # API 层
│       ├── handlers.h         # API 处理器接口
│       ├── handlers.cpp       # API 端点实现
│       ├── routes.h           # 路由定义接口
│       └── routes.cpp         # 路由设置和配置
├── include/                   # 公共头文件（如需要）
├── data/                      # 测试数据和配置文件
│   ├── grammars/             # 示例语法文件
│   └── test_inputs/          # 测试输入文件
├── build/                     # 构建目录（生成的）
├── CMakeLists.txt            # CMake 构建配置
├── package.json              # 项目元数据
└── README.md                 # 本文件
```

## 系统要求

- C++17 或更高版本
- CMake 3.10+
- Crow C++ 框架
- macOS、Linux 或 Windows（配备适当的编译器）

### 安装 Crow 框架

**macOS (Homebrew):**
```bash
brew install crowcpp/crow/crow
```

**Ubuntu:**
```bash
sudo apt-get install libcrow-dev
```

**从源码编译:**
```bash
git clone https://github.com/CrowCpp/Crow.git
cd Crow
mkdir build
cd build
cmake .. -DCROW_BUILD_EXAMPLES=OFF
make -j4
sudo make install
```

## 构建项目

1. 克隆或解压项目
2. 创建构建目录并编译：

```bash
mkdir -p build
cd build
cmake ..
make
```

## 运行服务器

```bash
cd build
./compiler_api
```

服务器将在 `http://localhost:8080` 启动

## API 端点

### 健康检查
- **GET** `/api/health`
- 返回服务器状态

### LR0 语法分析
- **POST** `/api/lr0/parse`
- 使用 LR0 语法分析器解析输入
- 请求体: `{"grammar": "E -> E + T | T\\nT -> (E) | a", "input": "a+a"}`

### 正则表达式自动机构建
- **POST** `/api/regex/build`
- 从正则表达式构建 NFA 和 DFA
- 请求体: `{"regex": "a*b+"}`

### 正则表达式字符串匹配
- **POST** `/api/regex/match`
- 测试输入字符串是否匹配正则表达式
- 请求体: `{"regex": "a*b+", "input": "aaabbb"}`

### 语法文件上传
- **POST** `/api/grammar/upload`
- 上传语法文件内容
- 请求体: `{"content": "E -> E + T | T\\nT -> (E) | a", "filename": "my_grammar.txt"}`

## 使用示例

```bash
# 健康检查
curl -X GET http://localhost:8080/api/health

# LR0 解析
curl -X POST http://localhost:8080/api/lr0/parse \\
  -H "Content-Type: application/json" \\
  -d '{"grammar": "E -> E + T | T\\nT -> (E) | a", "input": "a+a"}'

# 正则表达式匹配
curl -X POST http://localhost:8080/api/regex/match \\
  -H "Content-Type: application/json" \\
  -d '{"regex": "a*b+", "input": "aaabbb"}'
```

## 功能特性

- **超时保护**: 所有 API 端点都有可配置的超时限制
- **CORS 支持**: 支持跨域请求
- **错误处理**: 全面的错误处理，提供有意义的错误消息
- **请求验证**: 输入验证和清理
- **性能监控**: 请求计时和日志记录
- **模块化架构**: 核心逻辑与 API 层之间的清晰分离

## 架构设计

### 核心组件
- **LR0Parser**: 处理 LR0 语法分析和语法处理
- **RegexAutomata**: 管理正则表达式模式编译和字符串匹配

### API 层
- **APIHandlers**: 处理 HTTP 请求并格式化响应
- **APIRoutes**: 配置 URL 路由和 CORS 处理

### 配置
- 服务器超时: 30 秒
- 最大正则表达式长度: 1000 字符
- 最大输入字符串长度: 10,000 字符
- 并发线程数: 4

## 开发

### 添加新端点
1. 在 `src/api/handlers.cpp` 中添加处理器函数
2. 在 `src/api/routes.cpp` 中添加路由定义
3. 更新 API 文档

### 扩展核心功能
1. 在相应的核心模块（`lr0_parser.cpp` 或 `regex_automata.cpp`）中添加新函数
2. 更新相应的头文件
3. 在处理器中添加 API 接口

## 许可证

[在此添加许可证信息]

## 贡献

[在此添加贡献指南]
