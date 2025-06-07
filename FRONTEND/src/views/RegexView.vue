<template>
  <div class="regex-view">
    <div class="container">
      <header class="view-header">
        <h1>正则表达式自动机</h1>
        <p class="description">构建NFA/DFA自动机并进行字符串匹配</p>
      </header>

      <div class="main-content">
        <!-- 输入区域 -->
        <div class="input-section">
          <div class="tabs">
            <button
              @click="activeTab = 'build'"
              :class="['tab', { active: activeTab === 'build' }]"
            >
              构建自动机
            </button>
            <button
              @click="activeTab = 'match'"
              :class="['tab', { active: activeTab === 'match' }]"
            >
              字符串匹配
            </button>
          </div>

          <!-- 构建自动机标签页 -->
          <div v-if="activeTab === 'build'" class="tab-content">
            <div class="input-group">
              <label for="regex-build">正则表达式</label>
              <input
                id="regex-build"
                v-model="regexPattern"
                type="text"
                placeholder="请输入正则表达式，例如：(a|b)*abb"
                :disabled="store.isLoading"
              />
            </div>

            <div class="actions">
              <button
                @click="handleBuild"
                :disabled="!regexPattern.trim() || store.isLoading"
                class="btn-primary"
              >
                <span v-if="store.isLoading" class="loading-spinner"></span>
                {{ store.isLoading ? '构建中...' : '构建自动机' }}
              </button>
              <button
                @click="handleClear"
                :disabled="store.isLoading"
                class="btn-secondary"
              >
                清空
              </button>
            </div>
          </div>

          <!-- 字符串匹配标签页 -->
          <div v-if="activeTab === 'match'" class="tab-content">
            <div class="input-group">
              <label for="regex-match">正则表达式</label>
              <input
                id="regex-match"
                v-model="regexPattern"
                type="text"
                placeholder="请输入正则表达式，例如：(a|b)*abb"
                :disabled="store.isLoading"
              />
            </div>

            <div class="input-group">
              <label for="match-input">测试字符串</label>
              <input
                id="match-input"
                v-model="matchInput"
                type="text"
                placeholder="请输入要匹配的字符串"
                :disabled="store.isLoading"
              />
            </div>

            <div class="actions">
              <button
                @click="handleMatch"
                :disabled="!canMatch || store.isLoading"
                class="btn-primary"
              >
                <span v-if="store.isLoading" class="loading-spinner"></span>
                {{ store.isLoading ? '匹配中...' : '开始匹配' }}
              </button>
              <button
                @click="handleClear"
                :disabled="store.isLoading"
                class="btn-secondary"
              >
                清空
              </button>
            </div>
          </div>

          <!-- 快捷示例 -->
          <div class="examples">
            <h4>示例正则表达式</h4>
            <div class="example-buttons">
              <button
                v-for="example in examples"
                :key="example.regex"
                @click="loadExample(example)"
                class="example-btn"
                :disabled="store.isLoading"
              >
                {{ example.regex }}
              </button>
            </div>
          </div>
        </div>

        <!-- 结果区域 -->
        <div class="result-section" v-if="store.hasRegexResult || store.error">
          <!-- 错误提示 -->
          <div v-if="store.error" class="error-message">
            <h3>❌ 错误</h3>
            <p>{{ store.error }}</p>
            <button @click="store.clearError" class="btn-small">关闭</button>
          </div>

          <!-- 成功结果 -->
          <div v-else-if="store.regexResult" class="result-content">
            <div class="result-header">
              <h3>{{ store.isRegexSuccess ? '✅ 操作成功' : '❌ 操作失败' }}</h3>
              <p>{{ store.regexResult.message }}</p>
            </div>

            <!-- 匹配结果 -->
            <div v-if="hasPerformedMatch && store.regexResult.matchResult !== undefined" class="match-result">
              <div :class="['match-indicator', store.regexResult.matchResult ? 'matched' : 'not-matched']">
                {{ store.regexResult.matchResult ? '✅ 字符串匹配成功' : '❌ 字符串不匹配' }}
              </div>
            </div>

            <!-- 自动机描述和可视化 -->
            <div class="automata-section">
              <!-- 结果标签页 -->
              <div class="result-tabs">
                <button
                  @click="resultTab = 'description'"
                  :class="['tab', { active: resultTab === 'description' }]"
                >
                  文字描述
                </button>
                <button
                  @click="resultTab = 'visualization'"
                  :class="['tab', { active: resultTab === 'visualization' }]"
                  v-if="hasVisualization"
                >
                  图形化显示
                </button>
              </div>

              <!-- 文字描述标签页 -->
              <div v-if="resultTab === 'description'" class="tab-content">
                <!-- NFA描述 -->
                <div v-if="store.regexResult.nfaDescription" class="automata-item">
                  <h4>NFA (非确定有限自动机)</h4>
                  <pre class="automata-description">{{ store.regexResult.nfaDescription }}</pre>
                </div>

                <!-- DFA描述 -->
                <div v-if="store.regexResult.dfaDescription" class="automata-item">
                  <h4>DFA (确定有限自动机)</h4>
                  <pre class="automata-description">{{ store.regexResult.dfaDescription }}</pre>
                </div>
              </div>

              <!-- 可视化标签页 -->
              <div v-if="resultTab === 'visualization'" class="tab-content">
                <!-- 可视化子标签页 -->
                <div class="visualization-tabs">
                  <button
                    @click="vizTab = 'nfa'"
                    :class="['viz-tab', { active: vizTab === 'nfa' }]"
                    v-if="store.regexResult.nfaSvg"
                  >
                    NFA
                  </button>
                  <button
                    @click="vizTab = 'dfa'"
                    :class="['viz-tab', { active: vizTab === 'dfa' }]"
                    v-if="store.regexResult.dfaSvg"
                  >
                    DFA
                  </button>
                  <button
                    @click="vizTab = 'minimized'"
                    :class="['viz-tab', { active: vizTab === 'minimized' }]"
                    v-if="store.regexResult.minimizedDfaSvg"
                  >
                    最小化DFA
                  </button>
                </div>

                <!-- SVG 显示区域 -->
                <div class="svg-container">
                  <div v-if="vizTab === 'nfa' && store.regexResult.nfaSvg" class="svg-diagram">
                    <h4>NFA (非确定有限自动机)</h4>
                    <div v-html="store.regexResult.nfaSvg" class="svg-content"></div>
                  </div>
                  <div v-if="vizTab === 'dfa' && store.regexResult.dfaSvg" class="svg-diagram">
                    <h4>DFA (确定有限自动机)</h4>
                    <div v-html="store.regexResult.dfaSvg" class="svg-content"></div>
                  </div>
                  <div v-if="vizTab === 'minimized' && store.regexResult.minimizedDfaSvg" class="svg-diagram">
                    <h4>最小化DFA</h4>
                    <div v-html="store.regexResult.minimizedDfaSvg" class="svg-content"></div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>

        <!-- 连接状态 -->
        <div class="connection-status">
          <span :class="['status-indicator', store.isConnected ? 'connected' : 'disconnected']">
            {{ store.isConnected ? '🟢 服务器已连接' : '🔴 服务器未连接' }}
          </span>
          <button @click="store.checkConnection" class="btn-small" :disabled="store.isLoading">
            检查连接
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, watch } from 'vue'
import { useCompilerStore } from '../stores/compiler'

const store = useCompilerStore()

const activeTab = ref<'build' | 'match'>('build')
const regexPattern = ref('')
const matchInput = ref('')
const hasPerformedMatch = ref(false) // 跟踪是否执行过匹配操作

// 结果显示相关
const resultTab = ref<'description' | 'visualization'>('description')
const vizTab = ref<'nfa' | 'dfa' | 'minimized'>('nfa')

const examples = [
  { regex: 'a*', description: '零个或多个a' },
  { regex: '(a|b)*', description: 'a或b的零次或多次重复' },
  { regex: 'ab*', description: 'a后跟零个或多个b' },
  { regex: 'a(b|c)*', description: 'a后跟零个或多个b或c' },
]

const canMatch = computed(() => {
  return regexPattern.value.trim() !== '' && matchInput.value.trim() !== ''
})

const hasVisualization = computed(() => {
  return store.regexResult && (
    store.regexResult.nfaSvg ||
    store.regexResult.dfaSvg ||
    store.regexResult.minimizedDfaSvg
  )
})

const handleBuild = async () => {
  if (!regexPattern.value.trim()) return

  try {
    hasPerformedMatch.value = false // 重置匹配状态
    await store.buildRegex(regexPattern.value.trim())
  } catch (error) {
    // 错误已经在store中处理
  }
}

const handleMatch = async () => {
  if (!canMatch.value) return

  try {
    hasPerformedMatch.value = true // 标记已执行匹配
    await store.matchRegex(regexPattern.value.trim(), matchInput.value.trim())
  } catch (error) {
    // 错误已经在store中处理
  }
}

const handleClear = () => {
  regexPattern.value = ''
  matchInput.value = ''
  hasPerformedMatch.value = false // 重置匹配状态
  store.clearRegexResult()
}

const loadExample = (example: typeof examples[0]) => {
  regexPattern.value = example.regex
  if (activeTab.value === 'match' && !matchInput.value) {
    // 为匹配提供一些示例输入
    switch (example.regex) {
      case 'a*':
        matchInput.value = 'aaa'
        break
      case '(a|b)*':
        matchInput.value = 'ababab'
        break
      case 'ab*':
        matchInput.value = 'abbb'
        break
      case 'a(b|c)*':
        matchInput.value = 'abcbc'
        break
    }
  }
}

// 监听可视化数据变化，自动设置默认显示类型
watch(hasVisualization, (newVal) => {
  if (newVal && store.regexResult) {
    // 当有可视化数据时，自动切换到可视化标签页
    resultTab.value = 'visualization'

    // 设置默认显示的可视化类型（优先显示NFA）
    if (store.regexResult.nfaSvg) {
      vizTab.value = 'nfa'
    } else if (store.regexResult.dfaSvg) {
      vizTab.value = 'dfa'
    } else if (store.regexResult.minimizedDfaSvg) {
      vizTab.value = 'minimized'
    }
  }
})

onMounted(() => {
  // 检查服务器连接
  store.checkConnection()

  // 加载默认示例
  loadExample(examples[0])
})
</script>

<style scoped>
.regex-view {
  min-height: 100vh;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  padding: 2rem 1rem;
}

.container {
  max-width: 1200px;
  margin: 0 auto;
  background: white;
  border-radius: 16px;
  box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
  overflow: hidden;
}

.view-header {
  background: linear-gradient(135deg, #8e44ad 0%, #3498db 100%);
  color: white;
  padding: 3rem 2rem;
  text-align: center;
}

.view-header h1 {
  margin: 0 0 1rem 0;
  font-size: 2.5rem;
  font-weight: 700;
}

.description {
  margin: 0;
  font-size: 1.1rem;
  opacity: 0.9;
}

.main-content {
  padding: 2rem;
}

.tabs {
  display: flex;
  margin-bottom: 2rem;
  border-bottom: 1px solid #e1e8ed;
}

.tab {
  background: none;
  border: none;
  padding: 1rem 2rem;
  cursor: pointer;
  font-weight: 600;
  color: #7f8c8d;
  border-bottom: 2px solid transparent;
  transition: all 0.3s ease;
}

.tab.active {
  color: #8e44ad;
  border-bottom-color: #8e44ad;
}

.tab:hover {
  color: #8e44ad;
}

.tab-content {
  margin-bottom: 1.5rem;
}

.input-group {
  margin-bottom: 1.5rem;
}

.input-group label {
  display: block;
  margin-bottom: 0.5rem;
  font-weight: 600;
  color: #2c3e50;
}

.input-group input {
  width: 100%;
  padding: 0.75rem;
  border: 2px solid #e1e8ed;
  border-radius: 8px;
  font-family: 'Fira Code', monospace;
  font-size: 0.9rem;
  transition: border-color 0.3s ease;
}

.input-group input:focus {
  outline: none;
  border-color: #8e44ad;
}

.actions {
  display: flex;
  gap: 1rem;
  margin-top: 1.5rem;
}

.btn-primary {
  background: linear-gradient(135deg, #8e44ad 0%, #9b59b6 100%);
  color: white;
  border: none;
  padding: 0.75rem 2rem;
  border-radius: 8px;
  font-weight: 600;
  cursor: pointer;
  transition: transform 0.2s ease;
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.btn-primary:hover:not(:disabled) {
  transform: translateY(-2px);
}

.btn-primary:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

.btn-secondary {
  background: #95a5a6;
  color: white;
  border: none;
  padding: 0.75rem 1.5rem;
  border-radius: 8px;
  font-weight: 600;
  cursor: pointer;
  transition: background-color 0.3s ease;
}

.btn-secondary:hover:not(:disabled) {
  background: #7f8c8d;
}

.btn-small {
  background: #ecf0f1;
  color: #2c3e50;
  border: 1px solid #bdc3c7;
  padding: 0.25rem 0.75rem;
  border-radius: 4px;
  font-size: 0.8rem;
  cursor: pointer;
}

.loading-spinner {
  width: 16px;
  height: 16px;
  border: 2px solid transparent;
  border-top: 2px solid currentColor;
  border-radius: 50%;
  animation: spin 1s linear infinite;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

.examples {
  background: #f8f9fa;
  border-radius: 8px;
  padding: 1.5rem;
  margin-top: 1.5rem;
}

.examples h4 {
  margin: 0 0 1rem 0;
  color: #2c3e50;
}

.example-buttons {
  display: flex;
  flex-wrap: wrap;
  gap: 0.5rem;
}

.example-btn {
  background: white;
  border: 1px solid #8e44ad;
  color: #8e44ad;
  padding: 0.5rem 1rem;
  border-radius: 6px;
  cursor: pointer;
  font-family: 'Fira Code', monospace;
  font-size: 0.8rem;
  transition: all 0.3s ease;
}

.example-btn:hover:not(:disabled) {
  background: #8e44ad;
  color: white;
}

.result-section {
  border-top: 1px solid #e1e8ed;
  padding-top: 2rem;
}

.error-message {
  background: #fee;
  border: 1px solid #fcc;
  border-radius: 8px;
  padding: 1rem;
  margin-bottom: 1rem;
}

.error-message h3 {
  margin: 0 0 0.5rem 0;
  color: #c0392b;
}

.result-content {
  background: #f8f9fa;
  border-radius: 8px;
  padding: 1.5rem;
}

.result-header h3 {
  margin: 0 0 0.5rem 0;
  color: #2c3e50;
}

.match-result {
  margin: 1.5rem 0;
}

.match-indicator {
  padding: 1rem;
  border-radius: 8px;
  font-weight: 600;
  text-align: center;
}

.match-indicator.matched {
  background: #d4edda;
  color: #155724;
  border: 1px solid #c3e6cb;
}

.match-indicator.not-matched {
  background: #f8d7da;
  color: #721c24;
  border: 1px solid #f5c6cb;
}

.automata-section {
  margin-top: 1.5rem;
}

.automata-item {
  margin-bottom: 1.5rem;
}

.automata-item h4 {
  margin: 0 0 0.5rem 0;
  color: #2c3e50;
}

.automata-description {
  background: white;
  border: 1px solid #e1e8ed;
  border-radius: 6px;
  padding: 1rem;
  font-family: 'Fira Code', monospace;
  font-size: 0.8rem;
  overflow-x: auto;
  margin: 0;
  white-space: pre-wrap;
  word-wrap: break-word;
}

.connection-status {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-top: 2rem;
  padding: 1rem;
  background: #f8f9fa;
  border-radius: 8px;
  font-size: 0.9rem;
}

.status-indicator {
  font-weight: 600;
}

.status-indicator.connected {
  color: #27ae60;
}

.status-indicator.disconnected {
  color: #e74c3c;
}

/* 结果标签页样式 */
.result-tabs {
  display: flex;
  margin-bottom: 1.5rem;
  border-bottom: 1px solid #e1e8ed;
}

.result-tabs .tab {
  background: none;
  border: none;
  padding: 0.75rem 1.5rem;
  cursor: pointer;
  font-weight: 600;
  color: #7f8c8d;
  border-bottom: 2px solid transparent;
  transition: all 0.3s ease;
}

.result-tabs .tab.active {
  color: #27ae60;
  border-bottom-color: #27ae60;
}

.result-tabs .tab:hover {
  color: #27ae60;
}

/* 可视化标签页样式 */
.visualization-tabs {
  display: flex;
  margin-bottom: 1rem;
  gap: 0.5rem;
}

.viz-tab {
  background: #f8f9fa;
  border: 1px solid #e1e8ed;
  border-radius: 6px;
  padding: 0.5rem 1rem;
  cursor: pointer;
  font-weight: 500;
  color: #2c3e50;
  transition: all 0.3s ease;
}

.viz-tab.active {
  background: #8e44ad;
  color: white;
  border-color: #8e44ad;
}

.viz-tab:hover {
  background: #e9ecef;
  border-color: #8e44ad;
}

.viz-tab.active:hover {
  background: #9b59b6;
}

/* SVG 容器样式 */
.svg-container {
  margin-top: 1rem;
}

.svg-diagram {
  background: white;
  border: 1px solid #e1e8ed;
  border-radius: 8px;
  padding: 1.5rem;
  margin-bottom: 1rem;
}

.svg-diagram h4 {
  margin: 0 0 1rem 0;
  color: #2c3e50;
  text-align: center;
  font-size: 1.1rem;
}

.svg-content {
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 200px;
  overflow: auto;
}

.svg-content svg {
  max-width: 100%;
  height: auto;
  border-radius: 4px;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .visualization-tabs {
    flex-direction: column;
  }

  .viz-tab {
    text-align: center;
  }

  .svg-content {
    padding: 0.5rem;
  }
}
</style>
