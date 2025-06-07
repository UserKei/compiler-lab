<template>
  <div class="lr0-view">
    <div class="container">
      <header class="view-header">
        <h1>LR0 语法分析器</h1>
        <p class="description">输入上下文无关文法和待分析字符串，查看LR0分析过程</p>
      </header>

      <div class="main-content">
        <!-- 输入区域 -->
        <div class="input-section">
          <div class="input-group">
            <label for="grammar">文法规则</label>
            <textarea
              id="grammar"
              v-model="grammarInput"
              placeholder="请输入文法规则，例如：&#10;S -> A B&#10;A -> a&#10;B -> b"
              rows="8"
              :disabled="store.isLoading"
            ></textarea>
          </div>

          <div class="input-group">
            <label for="input">输入字符串</label>
            <input
              id="input"
              v-model="inputString"
              type="text"
              placeholder="请输入待分析的字符串"
              :disabled="store.isLoading"
            />
          </div>

          <div class="actions">
            <button
              @click="handleParse"
              :disabled="!canParse || store.isLoading"
              class="btn-primary"
            >
              <span v-if="store.isLoading" class="loading-spinner"></span>
              {{ store.isLoading ? '分析中...' : '开始分析' }}
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

        <!-- 结果区域 -->
        <div class="result-section" v-if="store.hasLR0Result || store.error">
          <!-- 错误提示 -->
          <div v-if="store.error" class="error-message">
            <h3>❌ 错误</h3>
            <p>{{ store.error }}</p>
            <button @click="store.clearError" class="btn-small">关闭</button>
          </div>

          <!-- 成功结果 -->
          <div v-else-if="store.lr0Result" class="result-content">
            <div class="result-header">
              <h3>{{ store.lr0Result.isAccepted ? '✅ 输入被接受' : '❌ 输入被拒绝' }}</h3>
              <p>{{ store.lr0Result.message }}</p>
            </div>

            <!-- 标签页导航 -->
            <div class="tab-navigation">
              <button
                @click="activeTab = 'steps'"
                :class="['tab-btn', { active: activeTab === 'steps' }]"
              >
                解析步骤
              </button>
              <button
                @click="activeTab = 'table'"
                :class="['tab-btn', { active: activeTab === 'table' }]"
              >
                分析表
              </button>
              <button
                @click="activeTab = 'diagram'"
                :class="['tab-btn', { active: activeTab === 'diagram' }]"
              >
                状态图
              </button>
              <button
                @click="activeTab = 'productions'"
                :class="['tab-btn', { active: activeTab === 'productions' }]"
              >
                产生式
              </button>
            </div>

            <!-- 标签页内容 -->
            <div class="tab-content">
              <!-- 解析步骤 -->
              <div v-if="activeTab === 'steps'" class="parse-steps-table">
                <h4>解析步骤</h4>
                <div class="table-container">
                  <table class="steps-table">
                    <thead>
                      <tr>
                        <th>步骤</th>
                        <th>状态栈</th>
                        <th>符号栈</th>
                        <th>剩余输入</th>
                        <th>动作</th>
                      </tr>
                    </thead>
                    <tbody>
                      <tr v-for="step in store.lr0Result.parseSteps" :key="step.step"
                          :class="{ 'error-step': step.action.includes('error'), 'accept-step': step.action === 'accept' }">
                        <td>{{ step.step }}</td>
                        <td class="monospace">{{ step.stateStack }}</td>
                        <td class="monospace">{{ step.symbolStack }}</td>
                        <td class="monospace">{{ step.remainingInput }}</td>
                        <td class="action-cell" :class="getActionClass(step.action)">{{ step.action }}</td>
                      </tr>
                    </tbody>
                  </table>
                </div>
              </div>

              <!-- 分析表 -->
              <div v-if="activeTab === 'table'" class="parse-table">
                <h4>LR0分析表</h4>

                <!-- 调试信息 -->
                <div v-if="false" class="debug-info" style="background: #f0f0f0; padding: 10px; margin: 10px 0; font-size: 12px;">
                  <p>Headers: {{ store.lr0Result?.parseTable?.headers }}</p>
                  <p>Terminal Headers: {{ getTerminalHeaders() }}</p>
                  <p>Nonterminal Headers: {{ getNonterminalHeaders() }}</p>
                  <p>Rows count: {{ store.lr0Result?.parseTable?.rows?.length }}</p>
                </div>

                <div class="table-container">
                  <table class="analysis-table">
                    <thead>
                      <tr>
                        <th rowspan="2">状态</th>
                        <th :colspan="getTerminalHeaders().length">ACTION</th>
                        <th :colspan="getNonterminalHeaders().length">GOTO</th>
                      </tr>
                      <tr>
                        <th v-for="header in getTerminalHeaders()" :key="'term-' + header">{{ header }}</th>
                        <th v-for="header in getNonterminalHeaders()" :key="'nonterm-' + header">{{ header }}</th>
                      </tr>
                    </thead>
                    <tbody>
                      <tr v-for="row in store.lr0Result.parseTable.rows" :key="row.state">
                        <td class="state-cell">{{ row.state }}</td>
                        <td v-for="terminal in getTerminalHeaders()" :key="'action-' + row.state + '-' + terminal"
                            :class="getTableCellClass(row.actions?.[terminal])">
                          {{ row.actions?.[terminal] || '' }}
                        </td>
                        <td v-for="nonterminal in getNonterminalHeaders()" :key="'goto-' + row.state + '-' + nonterminal"
                            class="goto-cell">
                          {{ row.gotos?.[nonterminal] !== undefined && row.gotos?.[nonterminal] !== null && row.gotos?.[nonterminal] !== -1 ? row.gotos[nonterminal] : '' }}
                        </td>
                      </tr>
                    </tbody>
                  </table>
                </div>
              </div>

              <!-- 状态转换图 -->
              <div v-if="activeTab === 'diagram'" class="state-diagram">
                <h4>状态转换图</h4>
                <div class="diagram-container">
                  <div v-if="store.lr0Result.svgDiagram" class="svg-display" v-html="store.lr0Result.svgDiagram"></div>
                  <div v-else class="no-diagram">
                    <p>无法生成状态转换图</p>
                    <small>请确保系统已安装Graphviz</small>
                  </div>
                </div>
              </div>

              <!-- 产生式 -->
              <div v-if="activeTab === 'productions'" class="productions">
                <h4>文法产生式</h4>
                <div v-if="store.lr0Result.productions && Object.keys(store.lr0Result.productions).length" class="production-list">
                  <template v-for="([left, rightsList], leftIndex) in Object.entries(store.lr0Result.productions)" :key="left">
                    <div
                      v-for="(rights, rightIndex) in rightsList"
                      :key="`${left}-${rightIndex}`"
                      class="production-item"
                    >
                      <span class="production-number">{{ leftIndex * 10 + rightIndex }}:</span>
                      <strong class="production-left">{{ left }}</strong>
                      <span class="production-arrow">→</span>
                      <span class="production-right">
                        <span v-for="(symbol, idx) in rights" :key="idx" class="right-symbol">
                          {{ symbol }}
                        </span>
                      </span>
                    </div>
                  </template>
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
import { ref, computed, onMounted } from 'vue'
import { useCompilerStore } from '../stores/compiler'

const store = useCompilerStore()

const grammarInput = ref('')
const inputString = ref('')
const activeTab = ref('steps')

// 示例数据
const exampleGrammar = `E -> E + T | T
T -> (E) | a`

const canParse = computed(() => {
  return grammarInput.value.trim() !== '' && inputString.value.trim() !== ''
})

const handleParse = async () => {
  if (!canParse.value) return

  try {
    await store.parseLR0(grammarInput.value.trim(), inputString.value.trim())
  } catch (error) {
    // 错误已经在store中处理
  }
}

const handleClear = () => {
  grammarInput.value = ''
  inputString.value = ''
  store.clearLR0Result()
}

const loadExample = () => {
  grammarInput.value = exampleGrammar
  inputString.value = '(a+a)'
}

// 获取终结符列头
const getTerminalHeaders = () => {
  if (!store.lr0Result?.parseTable?.headers) return []
  const headers = store.lr0Result.parseTable.headers
  const stateIndex = headers.indexOf('State')
  if (stateIndex === -1) return []

  const terminalHeaders = []
  for (let i = stateIndex + 1; i < headers.length; i++) {
    const header = headers[i]
    // 终结符：不是以大写字母开头的符号（即小写字母、数字、特殊符号）
    if (!header.match(/^[A-Z]/)) {
      terminalHeaders.push(header)
    } else {
      break // 遇到非终结符就停止，因为终结符在前，非终结符在后
    }
  }
  return terminalHeaders
}

// 获取非终结符列头
const getNonterminalHeaders = () => {
  if (!store.lr0Result?.parseTable?.headers) return []
  const headers = store.lr0Result.parseTable.headers
  const terminalCount = getTerminalHeaders().length
  const stateIndex = headers.indexOf('State')
  if (stateIndex === -1) return []

  return headers.slice(stateIndex + 1 + terminalCount)
}

// 获取动作的CSS类
const getActionClass = (action: string) => {
  if (action.includes('error')) return 'error-action'
  if (action === 'accept') return 'accept-action'
  if (action.startsWith('shift')) return 'shift-action'
  if (action.startsWith('reduce')) return 'reduce-action'
  return ''
}

// 获取表格单元格的CSS类
const getTableCellClass = (value: string) => {
  if (!value) return 'empty-cell'
  if (value === 'acc') return 'accept-cell'
  if (value.startsWith('s')) return 'shift-cell'
  if (value.startsWith('r')) return 'reduce-cell'
  return ''
}

onMounted(() => {
  // 检查服务器连接
  store.checkConnection()

  // 加载示例
  loadExample()
})
</script>

<style scoped>
.lr0-view {
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
  background: linear-gradient(135deg, #2c3e50 0%, #3498db 100%);
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

.input-section {
  margin-bottom: 2rem;
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

.input-group textarea,
.input-group input {
  width: 100%;
  padding: 0.75rem;
  border: 2px solid #e1e8ed;
  border-radius: 8px;
  font-family: 'Fira Code', monospace;
  font-size: 0.9rem;
  transition: border-color 0.3s ease;
}

.input-group textarea:focus,
.input-group input:focus {
  outline: none;
  border-color: #3498db;
}

.input-group textarea {
  resize: vertical;
  min-height: 120px;
}

.actions {
  display: flex;
  gap: 1rem;
  margin-top: 1.5rem;
}

.btn-primary {
  background: linear-gradient(135deg, #3498db 0%, #2980b9 100%);
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

.parse-steps {
  margin-top: 1.5rem;
}

.parse-steps h4 {
  margin: 0 0 1rem 0;
  color: #2c3e50;
}

.parse-steps ol {
  background: white;
  border-radius: 6px;
  padding: 1rem;
  margin: 0;
}

.productions {
  margin-top: 1.5rem;
}

.productions h4 {
  margin: 0 0 1rem 0;
  color: #2c3e50;
}

.production-list {
  background: white;
  border-radius: 6px;
  padding: 1rem;
}

.production-item {
  display: flex;
  align-items: center;
  margin: 0.75rem 0;
  padding: 0.5rem;
  background: #f8f9fa;
  border-radius: 6px;
  font-family: 'Fira Code', monospace;
}

.production-number {
  background: #3498db;
  color: white;
  padding: 0.25rem 0.5rem;
  border-radius: 4px;
  font-weight: 600;
  margin-right: 1rem;
  min-width: 2rem;
  text-align: center;
}

.production-left {
  color: #2c3e50;
  margin-right: 0.5rem;
}

.production-arrow {
  color: #7f8c8d;
  margin: 0 0.5rem;
  font-weight: 600;
}

.production-right {
  color: #e74c3c;
}

.right-symbol {
  margin-right: 0.5rem;
}

/* 标签页导航 */
.tab-navigation {
  display: flex;
  border-bottom: 2px solid #e1e8ed;
  margin-bottom: 1.5rem;
  gap: 0.5rem;
}

.tab-btn {
  background: none;
  border: none;
  padding: 1rem 1.5rem;
  font-weight: 600;
  color: #7f8c8d;
  cursor: pointer;
  border-bottom: 3px solid transparent;
  transition: all 0.3s ease;
}

.tab-btn:hover {
  color: #3498db;
}

.tab-btn.active {
  color: #3498db;
  border-bottom-color: #3498db;
}

/* 表格样式 */
.table-container {
  overflow-x: auto;
  background: white;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
}

.steps-table, .analysis-table {
  width: 100%;
  border-collapse: collapse;
  font-family: 'Fira Code', monospace;
  font-size: 0.85rem;
}

.steps-table th, .analysis-table th,
.steps-table td, .analysis-table td {
  padding: 0.75rem;
  text-align: center;
  border: 1px solid #e1e8ed;
}

.steps-table th, .analysis-table th {
  background: #f8f9fa;
  font-weight: 600;
  color: #2c3e50;
}

.steps-table tbody tr:hover {
  background: #f8f9fa;
}

.monospace {
  font-family: 'Fira Code', monospace;
}

/* 解析步骤行样式 */
.error-step {
  background-color: #fee !important;
}

.accept-step {
  background-color: #eef !important;
}

/* 动作单元格样式 */
.action-cell {
  font-weight: 600;
}

.error-action {
  color: #e74c3c;
}

.accept-action {
  color: #27ae60;
}

.shift-action {
  color: #3498db;
}

.reduce-action {
  color: #f39c12;
}

/* 分析表单元格样式 */
.state-cell {
  font-weight: 600;
  background: #ecf0f1;
}

.empty-cell {
  color: #bdc3c7;
}

.accept-cell {
  background-color: #d5f4e6;
  color: #27ae60;
  font-weight: 600;
}

.shift-cell {
  background-color: #e3f2fd;
  color: #1976d2;
}

.reduce-cell {
  background-color: #fff3e0;
  color: #f57c00;
}

.goto-cell {
  background-color: #f3e5f5;
  color: #7b1fa2;
}

/* SVG图表显示 */
.diagram-container {
  background: white;
  border-radius: 8px;
  padding: 1rem;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
}

.svg-display {
  text-align: center;
}

.svg-display svg {
  max-width: 100%;
  height: auto;
}

.no-diagram {
  text-align: center;
  padding: 2rem;
  color: #7f8c8d;
}

.no-diagram p {
  margin: 0 0 0.5rem 0;
  font-size: 1.1rem;
}

.no-diagram small {
  font-size: 0.9rem;
}

/* 标签页内容 */
.tab-content {
  min-height: 300px;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .tab-navigation {
    flex-wrap: wrap;
  }

  .tab-btn {
    padding: 0.75rem 1rem;
    font-size: 0.9rem;
  }

  .steps-table, .analysis-table {
    font-size: 0.75rem;
  }

  .steps-table th, .analysis-table th,
  .steps-table td, .analysis-table td {
    padding: 0.5rem;
  }
}
</style>
