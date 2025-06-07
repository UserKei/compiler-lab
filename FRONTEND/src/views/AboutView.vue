<template>
  <div class="about-view">
    <div class="container">
      <header class="about-header">
        <h1>关于编译器工具集</h1>
        <p class="subtitle">专为编译原理学习设计的在线工具平台</p>
      </header>

      <div class="content-section">
        <div class="section">
          <h2>🎯 项目目标</h2>
          <p>
            编译器工具集旨在为学习编译原理的学生和开发者提供直观、易用的在线工具。
            通过可视化的方式展示编译过程中的关键概念，帮助理解抽象的理论知识。
          </p>
        </div>

        <div class="section">
          <h2>⚡ 技术架构</h2>
          <div class="tech-stack">
            <div class="tech-item">
              <div class="tech-icon frontend">🌐</div>
              <div class="tech-details">
                <h3>前端技术</h3>
                <ul>
                  <li>Vue 3 - 渐进式JavaScript框架</li>
                  <li>TypeScript - 类型安全的JavaScript</li>
                  <li>Vite - 快速的构建工具</li>
                  <li>Pinia - 状态管理库</li>
                </ul>
              </div>
            </div>

            <div class="tech-item">
              <div class="tech-icon backend">⚙️</div>
              <div class="tech-details">
                <h3>后端技术</h3>
                <ul>
                  <li>C++ - 高性能编程语言</li>
                  <li>Crow - 轻量级Web框架</li>
                  <li>CMake - 跨平台构建系统</li>
                  <li>RESTful API - 标准化接口设计</li>
                </ul>
              </div>
            </div>
          </div>
        </div>

        <div class="section">
          <h2>🔧 功能特性</h2>
          <div class="features-grid">
            <div class="feature-item">
              <h3>LR0 语法分析器</h3>
              <p>支持上下文无关文法的LR0分析，展示分析表构建过程和字符串解析步骤。</p>
            </div>

            <div class="feature-item">
              <h3>正则表达式自动机</h3>
              <p>从正则表达式构建NFA和DFA，支持字符串匹配测试和状态转换可视化。</p>
            </div>

            <div class="feature-item">
              <h3>实时处理</h3>
              <p>基于C++的高性能后端，确保复杂算法的快速执行和实时响应。</p>
            </div>

            <div class="feature-item">
              <h3>用户友好</h3>
              <p>简洁直观的界面设计，提供丰富的示例和详细的结果展示。</p>
            </div>
          </div>
        </div>

        <div class="section">
          <h2>📈 开发进度</h2>
          <div class="progress-list">
            <div class="progress-item completed">
              <span class="status">✅</span>
              <span>项目架构搭建</span>
            </div>
            <div class="progress-item completed">
              <span class="status">✅</span>
              <span>前端界面开发</span>
            </div>
            <div class="progress-item completed">
              <span class="status">✅</span>
              <span>后端API设计</span>
            </div>
            <div class="progress-item in-progress">
              <span class="status">🔄</span>
              <span>LR0分析算法实现</span>
            </div>
            <div class="progress-item in-progress">
              <span class="status">🔄</span>
              <span>正则自动机算法完善</span>
            </div>
            <div class="progress-item planned">
              <span class="status">📋</span>
              <span>算法可视化增强</span>
            </div>
            <div class="progress-item planned">
              <span class="status">📋</span>
              <span>更多编译工具集成</span>
            </div>
          </div>
        </div>

        <div class="section">
          <h2>💡 使用建议</h2>
          <div class="tips-container">
            <div class="tip-item">
              <strong>学习路径：</strong>
              建议先从简单的正则表达式开始，理解自动机的基本概念，然后进阶到LR0语法分析。
            </div>
            <div class="tip-item">
              <strong>实践方法：</strong>
              尝试输入不同的文法和正则表达式，观察算法的执行过程，加深对理论的理解。
            </div>
            <div class="tip-item">
              <strong>问题反馈：</strong>
              如遇到问题或有改进建议，欢迎通过GitHub Issues或邮件联系我们。
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.about-view {
  min-height: 100vh;
  background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
  padding: 2rem 1rem;
}

.container {
  max-width: 1000px;
  margin: 0 auto;
  background: white;
  border-radius: 16px;
  box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
  overflow: hidden;
}

.about-header {
  background: linear-gradient(135deg, #2c3e50 0%, #34495e 100%);
  color: white;
  padding: 3rem 2rem;
  text-align: center;
}

.about-header h1 {
  margin: 0 0 1rem 0;
  font-size: 2.5rem;
  font-weight: 700;
}

.subtitle {
  margin: 0;
  font-size: 1.2rem;
  opacity: 0.9;
}

.content-section {
  padding: 2rem;
}

.section {
  margin-bottom: 3rem;
}

.section h2 {
  color: #2c3e50;
  font-size: 1.8rem;
  margin: 0 0 1.5rem 0;
  padding-bottom: 0.5rem;
  border-bottom: 2px solid #3498db;
}

.section p {
  color: #7f8c8d;
  line-height: 1.8;
  font-size: 1.1rem;
}

.tech-stack {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
  gap: 2rem;
  margin-top: 1.5rem;
}

.tech-item {
  display: flex;
  gap: 1.5rem;
  background: #f8f9fa;
  padding: 1.5rem;
  border-radius: 12px;
  border-left: 4px solid #3498db;
}

.tech-icon {
  font-size: 2.5rem;
  flex-shrink: 0;
}

.tech-icon.frontend {
  color: #3498db;
}

.tech-icon.backend {
  color: #e67e22;
}

.tech-details h3 {
  margin: 0 0 1rem 0;
  color: #2c3e50;
  font-size: 1.3rem;
}

.tech-details ul {
  margin: 0;
  padding-left: 1.5rem;
  color: #7f8c8d;
}

.tech-details li {
  margin-bottom: 0.5rem;
  line-height: 1.6;
}

.features-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 1.5rem;
  margin-top: 1.5rem;
}

.feature-item {
  background: #f8f9fa;
  padding: 1.5rem;
  border-radius: 12px;
  border-top: 4px solid #27ae60;
}

.feature-item h3 {
  margin: 0 0 1rem 0;
  color: #2c3e50;
  font-size: 1.2rem;
}

.feature-item p {
  margin: 0;
  color: #7f8c8d;
  font-size: 0.95rem;
  line-height: 1.6;
}

.progress-list {
  display: flex;
  flex-direction: column;
  gap: 1rem;
  margin-top: 1.5rem;
}

.progress-item {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem;
  background: #f8f9fa;
  border-radius: 8px;
  transition: transform 0.2s ease;
}

.progress-item:hover {
  transform: translateX(4px);
}

.progress-item.completed {
  border-left: 4px solid #27ae60;
}

.progress-item.in-progress {
  border-left: 4px solid #f39c12;
}

.progress-item.planned {
  border-left: 4px solid #95a5a6;
}

.status {
  font-size: 1.2rem;
  flex-shrink: 0;
}

.tips-container {
  display: flex;
  flex-direction: column;
  gap: 1rem;
  margin-top: 1.5rem;
}

.tip-item {
  background: #e8f4f8;
  padding: 1.5rem;
  border-radius: 8px;
  border-left: 4px solid #3498db;
  line-height: 1.6;
  color: #2c3e50;
}

.tip-item strong {
  color: #2980b9;
}

@media (max-width: 768px) {
  .about-header h1 {
    font-size: 2rem;
  }

  .tech-stack {
    grid-template-columns: 1fr;
  }

  .tech-item {
    flex-direction: column;
    text-align: center;
  }

  .features-grid {
    grid-template-columns: 1fr;
  }
}
</style>
