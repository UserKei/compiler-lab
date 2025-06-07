<script setup lang="ts">
import { RouterLink, RouterView } from 'vue-router'
</script>

<template>
  <div id="app">
    <nav class="navbar">
      <div class="nav-container">
        <RouterLink to="/" class="nav-brand">
          <span class="brand-icon">⚙️</span>
          编译器工具集
        </RouterLink>

        <div class="nav-links">
          <RouterLink to="/" class="nav-link">首页</RouterLink>
          <RouterLink to="/lr0" class="nav-link">LR0分析</RouterLink>
          <RouterLink to="/regex" class="nav-link">正则自动机</RouterLink>
          <RouterLink to="/about" class="nav-link">关于</RouterLink>
        </div>
      </div>
    </nav>

    <main class="main-content">
      <RouterView />
    </main>

    <footer class="footer">
      <div class="footer-container">
        <p>&copy; 2025 编译器工具集. 使用 Vue 3 + TypeScript + C++ 构建</p>
      </div>
    </footer>
  </div>
</template>

<style scoped>
#app {
  min-height: 100vh;
  display: flex;
  flex-direction: column;
}

.navbar {
  background: rgba(255, 255, 255, 0.95);
  backdrop-filter: blur(10px);
  border-bottom: 1px solid rgba(0, 0, 0, 0.1);
  position: sticky;
  top: 0;
  z-index: 100;
}

.nav-container {
  max-width: 1200px;
  margin: 0 auto;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 1rem 2rem;
}

.nav-brand {
  font-size: 1.5rem;
  font-weight: 700;
  color: #2c3e50;
  text-decoration: none;
  display: flex;
  align-items: center;
  gap: 0.5rem;
  transition: color 0.3s ease;
}

.nav-brand:hover {
  color: #3498db;
}

.brand-icon {
  font-size: 1.25rem;
}

.nav-links {
  display: flex;
  gap: 2rem;
}

.nav-link {
  color: #7f8c8d;
  text-decoration: none;
  font-weight: 500;
  padding: 0.5rem 1rem;
  border-radius: 8px;
  transition: all 0.3s ease;
}

.nav-link:hover {
  color: #2c3e50;
  background: rgba(52, 152, 219, 0.1);
}

.nav-link.router-link-active {
  color: #3498db;
  background: rgba(52, 152, 219, 0.1);
}

.main-content {
  flex: 1;
}

.footer {
  background: #2c3e50;
  color: white;
  text-align: center;
  padding: 2rem 1rem;
  margin-top: auto;
}

.footer-container {
  max-width: 1200px;
  margin: 0 auto;
}

.footer p {
  margin: 0;
  opacity: 0.8;
}

@media (max-width: 768px) {
  .nav-container {
    flex-direction: column;
    gap: 1rem;
    padding: 1rem;
  }

  .nav-links {
    gap: 1rem;
  }

  .nav-link {
    padding: 0.5rem;
    font-size: 0.9rem;
  }
}
</style>
