# 编译器工具集

一个基于 Vue.js + TypeScript (前端) 和 C++ (后端) 的编译器工具可视化项目。

## 功能特性

- **LR0 语法分析器**: 上下文无关文法的LR0分析过程可视化
- **正则表达式自动机**: 正则表达式到NFA/DFA的转换和匹配可视化
- **实时交互**: 响应式界面，实时显示分析结果
- **RESTful API**: 后端提供标准的REST API接口

## 技术栈

### 前端
- Vue.js 3
- TypeScript
- Vite
- Pinia (状态管理)
- Vue Router

### 后端
- C++17
- CMake
- Crow Web Framework
- 自研编译器算法实现

## 项目结构

```
├── FRONTEND/          # Vue.js 前端项目
│   ├── src/
│   │   ├── views/     # 页面组件
│   │   ├── services/  # API服务
│   │   ├── stores/    # 状态管理
│   │   └── router/    # 路由配置
│   └── package.json
├── BACKEND/           # C++ 后端项目
│   ├── src/
│   │   ├── core/      # 核心算法实现
│   │   └── api/       # Web API接口
│   ├── data/          # 测试数据
│   └── CMakeLists.txt
└── README.md
```

## 快速开始

### 环境要求

- Node.js 16+
- C++17 编译器 (GCC/Clang)
- CMake 3.16+

### 启动后端

```bash
cd BACKEND
mkdir build && cd build
cmake ..
make
./compiler_api
```

后端将在 `http://localhost:8080` 启动

### 启动前端

```bash
cd FRONTEND
npm install
npm run dev
```

前端将在 `http://localhost:5173` 启动

## API 文档

### 健康检查
- **GET** `/api/health` - 检查服务器状态

### LR0 语法分析
- **POST** `/api/lr0/parse` - 执行LR0语法分析
  ```json
  {
    "grammar": "文法规则",
    "input": "输入字符串"
  }
  ```

### 正则表达式
- **POST** `/api/regex/build` - 构建正则表达式自动机
  ```json
  {
    "regex": "正则表达式模式"
  }
  ```

- **POST** `/api/regex/match` - 正则表达式匹配
  ```json
  {
    "regex": "正则表达式模式",
    "input": "待匹配字符串"
  }
  ```

## 开发指南

### 前端开发
```bash
cd FRONTEND
npm run dev      # 开发服务器
npm run build    # 构建生产版本
npm run preview  # 预览生产版本
```

### 后端开发
```bash
cd BACKEND/build
make             # 编译
./compiler_api   # 运行服务器
```

## 项目特色

- **CORS支持**: 后端已配置跨域资源共享
- **代理配置**: 前端Vite配置了API代理，避免跨域问题
- **错误处理**: 完善的错误处理和用户反馈
- **响应式设计**: 现代化的用户界面

## 贡献指南

1. Fork 项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 许可证

MIT License

## 作者

Kei - 编译器工具集开发者

---

*构建时间: 2025年6月*
