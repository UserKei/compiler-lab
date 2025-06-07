<script setup lang="ts">
import { onMounted } from 'vue'
import { RouterLink } from 'vue-router'
import { useCompilerStore } from '../stores/compiler'

const store = useCompilerStore()

onMounted(() => {
  store.checkConnection()
})
</script>

<template>
  <main class="home-view">
    <div class="hero-section">
      <div class="container">
        <h1 class="hero-title">编译器工具集</h1>
        <p class="hero-subtitle">
          强大的编译原理学习工具，支持LR0语法分析和正则表达式自动机构建
        </p>

        <div class="connection-check">
          <div :class="['status-badge', store.isConnected ? 'connected' : 'disconnected']">
            {{ store.isConnected ? '🟢 服务器已连接' : '🔴 服务器连接中...' }}
          </div>
        </div>
      </div>
    </div>

    <div class="features-section">
      <div class="container">
        <h2 class="section-title">功能特性</h2>

        <div class="features-grid">
          <div class="feature-card">
            <div class="feature-icon">📝</div>
            <h3>LR0 语法分析</h3>
            <p>输入上下文无关文法，分析字符串的语法结构，查看详细的分析步骤和产生式应用过程</p>
            <RouterLink to="/lr0" class="feature-link">
              立即体验 →
            </RouterLink>
          </div>

          <div class="feature-card">
            <div class="feature-icon">🔄</div>
            <h3>正则表达式自动机</h3>
            <p>构建NFA和DFA自动机，进行字符串匹配测试，可视化自动机的状态转换过程</p>
            <RouterLink to="/regex" class="feature-link">
              立即体验 →
            </RouterLink>
          </div>

          <div class="feature-card">
            <div class="feature-icon">⚡</div>
            <h3>实时处理</h3>
            <p>基于C++后端的高性能处理引擎，支持复杂文法和正则表达式的快速分析</p>
            <a href="#" class="feature-link disabled">
              敬请期待
            </a>
          </div>
        </div>
      </div>
    </div>

    <div class="getting-started-section">
      <div class="container">
        <h2 class="section-title">快速开始</h2>

        <div class="steps-container">
          <div class="step-item">
            <div class="step-number">1</div>
            <div class="step-content">
              <h3>选择工具</h3>
              <p>根据需要选择LR0语法分析器或正则表达式自动机工具</p>
            </div>
          </div>

          <div class="step-item">
            <div class="step-number">2</div>
            <div class="step-content">
              <h3>输入规则</h3>
              <p>输入文法规则或正则表达式，系统提供了多个示例供参考</p>
            </div>
          </div>

          <div class="step-item">
            <div class="step-number">3</div>
            <div class="step-content">
              <h3>查看结果</h3>
              <p>获得详细的分析结果，包括步骤说明和可视化展示</p>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- 快速演示区域 -->
    <!-- <div class="demo-section">
      <div class="container">
        <QuickDemo />
      </div>
    </div> -->
  </main>
</template>

<style scoped>
.home-view {
  min-height: 100vh;
}

.hero-section {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  padding: 4rem 1rem;
  text-align: center;
}

.container {
  max-width: 1200px;
  margin: 0 auto;
}

.hero-title {
  font-size: 3.5rem;
  font-weight: 700;
  margin: 0 0 1rem 0;
  text-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
}

.hero-subtitle {
  font-size: 1.25rem;
  margin: 0 0 2rem 0;
  opacity: 0.9;
  max-width: 600px;
  margin-left: auto;
  margin-right: auto;
  line-height: 1.6;
}

.connection-check {
  margin-top: 2rem;
}

.status-badge {
  display: inline-block;
  padding: 0.75rem 1.5rem;
  border-radius: 50px;
  font-weight: 600;
  font-size: 0.9rem;
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255, 255, 255, 0.2);
}

.status-badge.connected {
  background: rgba(46, 204, 113, 0.2);
  color: #2ecc71;
}

.status-badge.disconnected {
  background: rgba(231, 76, 60, 0.2);
  color: #e74c3c;
}

.features-section {
  padding: 4rem 1rem;
  background: #f8f9fa;
}

.section-title {
  text-align: center;
  font-size: 2.5rem;
  font-weight: 700;
  margin: 0 0 3rem 0;
  color: #2c3e50;
}

.features-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 2rem;
  max-width: 1000px;
  margin: 0 auto;
}

.feature-card {
  background: white;
  border-radius: 16px;
  padding: 2rem;
  text-align: center;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
  transition: transform 0.3s ease, box-shadow 0.3s ease;
}

.feature-card:hover {
  transform: translateY(-8px);
  box-shadow: 0 20px 40px rgba(0, 0, 0, 0.15);
}

.feature-icon {
  font-size: 3rem;
  margin-bottom: 1rem;
}

.feature-card h3 {
  font-size: 1.5rem;
  font-weight: 600;
  margin: 0 0 1rem 0;
  color: #2c3e50;
}

.feature-card p {
  color: #7f8c8d;
  line-height: 1.6;
  margin: 0 0 1.5rem 0;
}

.feature-link {
  display: inline-block;
  background: linear-gradient(135deg, #3498db 0%, #2980b9 100%);
  color: white;
  text-decoration: none;
  padding: 0.75rem 1.5rem;
  border-radius: 8px;
  font-weight: 600;
  transition: transform 0.2s ease;
}

.feature-link:hover:not(.disabled) {
  transform: translateY(-2px);
}

.feature-link.disabled {
  background: #bdc3c7;
  cursor: not-allowed;
}

.getting-started-section {
  padding: 4rem 1rem;
  background: white;
}

.steps-container {
  display: flex;
  flex-direction: column;
  gap: 2rem;
  max-width: 800px;
  margin: 0 auto;
}

.step-item {
  display: flex;
  align-items: center;
  gap: 2rem;
  padding: 2rem;
  background: #f8f9fa;
  border-radius: 16px;
  transition: transform 0.3s ease;
}

.step-item:hover {
  transform: translateX(8px);
}

.step-number {
  flex-shrink: 0;
  width: 60px;
  height: 60px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 1.5rem;
  font-weight: 700;
}

.step-content h3 {
  font-size: 1.25rem;
  font-weight: 600;
  margin: 0 0 0.5rem 0;
  color: #2c3e50;
}

.step-content p {
  margin: 0;
  color: #7f8c8d;
  line-height: 1.6;
}

.demo-section {
  padding: 2rem 1rem;
  background: #ecf0f1;
}

@media (max-width: 768px) {
  .hero-title {
    font-size: 2.5rem;
  }

  .hero-subtitle {
    font-size: 1.1rem;
  }

  .step-item {
    flex-direction: column;
    text-align: center;
    gap: 1rem;
  }
}
</style>
