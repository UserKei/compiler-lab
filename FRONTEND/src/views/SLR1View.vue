<template>
  <div class="slr1-view">
    <div class="container">
      <header class="view-header">
        <h1>SLR(1) 语法分析器</h1>
        <p class="description">SLR(1) 是 LR(0) 的改进版本，使用 FOLLOW 集合来解决 LR(0) 的冲突问题</p>
      </header>

      <div class="main-content">
        <!-- 输入区域 -->
        <div class="input-section">
          <div class="input-group">
            <label for="grammar">文法规则</label>
            <textarea id="grammar" v-model="grammarInput" placeholder="请输入文法规则，例如：&#10;S -> A B&#10;A -> a&#10;B -> b"
              rows="8" :disabled="store.isLoading"></textarea>
          </div>

          <div class="input-group">
            <label for="input">输入字符串</label>
            <input id="input" v-model="inputString" type="text" placeholder="请输入待分析的字符串" :disabled="store.isLoading" />
          </div>

          <div class="actions">
            <button @click="handleParse" :disabled="!canParse || store.isLoading" class="btn-primary">
              <span v-if="store.isLoading" class="loading-spinner"></span>
              {{ store.isLoading ? '分析中...' : '开始分析' }}
            </button>
            <button @click="handleClear" :disabled="store.isLoading" class="btn-secondary">
              清空
            </button>
          </div>
        </div>

        <!-- 结果区域 -->
        <div class="result-section" v-if="store.hasSLR1Result || store.error">
          <!-- 错误提示 -->
          <div v-if="store.error" class="error-message">
            <h3>❌ 错误</h3>
            <p>{{ store.error }}</p>
            <button @click="store.clearError" class="btn-small">关闭</button>
          </div>

          <!-- 成功结果 -->
          <div v-else-if="store.slr1Result" class="result-content">
            <div class="result-header">
              <h3>{{ store.slr1Result.isAccepted ? '✅ 输入被接受' : '❌ 输入被拒绝' }}</h3>
              <p>{{ store.slr1Result.message }}</p>
            </div>

            <!-- 标签页导航 -->
            <div class="tab-navigation">
              <button @click="activeTab = 'steps'" :class="['tab-btn', { active: activeTab === 'steps' }]">
                解析步骤
              </button>
              <button @click="activeTab = 'table'" :class="['tab-btn', { active: activeTab === 'table' }]">
                分析表
              </button>
              <button @click="activeTab = 'diagram'" :class="['tab-btn', { active: activeTab === 'diagram' }]">
                状态图
              </button>
              <button @click="activeTab = 'sets'" :class="['tab-btn', { active: activeTab === 'sets' }]">
                FIRST/FOLLOW集合
              </button>
              <button @click="activeTab = 'productions'" :class="['tab-btn', { active: activeTab === 'productions' }]">
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
                      <tr v-for="step in store.slr1Result.parseSteps" :key="step.step"
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
                <h4>SLR(1)分析表</h4>
                <div class="table-container">
                  <div class="table-wrapper">
                    <table class="analysis-table">
                      <thead>
                        <tr>
                          <th rowspan="2" class="state-header">状态</th>
                          <th colspan="4" class="action-header">ACTION</th>
                          <th colspan="3" class="goto-header">GOTO</th>
                        </tr>
                        <tr>
                          <th v-for="terminal in getTerminals()" :key="terminal">{{ terminal }}</th>
                          <th v-for="nonterminal in getNonterminals()" :key="nonterminal">{{ nonterminal }}</th>
                        </tr>
                      </thead>
                      <tbody>
                        <tr v-for="row in store.slr1Result.parseTable.rows" :key="row.state">
                          <td class="state-cell">{{ row.state }}</td>
                          <td v-for="terminal in getTerminals()" :key="terminal" class="action-cell"
                            :class="getActionCellClass(row.actions[terminal])">
                            {{ row.actions[terminal] || '' }}
                          </td>
                          <td v-for="nonterminal in getNonterminals()" :key="nonterminal" class="goto-cell">
                            {{ (row.gotos[nonterminal] !== undefined && row.gotos[nonterminal] !== -1) ?
                              row.gotos[nonterminal] : '' }}
                          </td>
                        </tr>
                      </tbody>
                    </table>
                  </div>
                </div>
              </div>

              <!-- 状态图 -->
              <div v-if="activeTab === 'diagram'" class="automaton-diagram">
                <h4>SLR(1) 自动机状态图</h4>
                <div v-if="store.slr1Result.svgDiagram" class="svg-container" v-html="store.slr1Result.svgDiagram">
                </div>
                <div v-else class="no-diagram">
                  <p>自动机图生成中...</p>
                  <p class="hint">状态图将显示 SLR(1) 自动机的状态转换关系</p>
                </div>
              </div>

              <!-- FIRST/FOLLOW集合 -->
              <div v-if="activeTab === 'sets'" class="sets-section">
                <div class="sets-container">
                  <div class="first-sets">
                    <h4>FIRST 集合</h4>
                    <div class="set-list">
                      <div v-for="(symbols, nonterminal) in store.slr1Result.firstSets" :key="nonterminal"
                        class="set-item">
                        <strong>FIRST({{ nonterminal }}) =</strong>
                        <span class="set-symbols">{ {{ Array.isArray(symbols) ? symbols.join(', ') :
                          Object.keys(symbols).join(', ') }} }</span>
                      </div>
                    </div>
                  </div>

                  <div class="follow-sets">
                    <h4>FOLLOW 集合</h4>
                    <div class="set-list">
                      <div v-for="(symbols, nonterminal) in store.slr1Result.followSets" :key="nonterminal"
                        class="set-item">
                        <strong>FOLLOW({{ nonterminal }}) =</strong>
                        <span class="set-symbols">{ {{ Array.isArray(symbols) ? symbols.join(', ') :
                          Object.keys(symbols).join(', ') }} }</span>
                      </div>
                    </div>
                  </div>
                </div>
              </div>

              <!-- 产生式 -->
              <div v-if="activeTab === 'productions'" class="productions">
                <h4>文法产生式</h4>

                <div v-if="store.slr1Result?.productionList && store.slr1Result.productionList.length > 0"
                  class="production-list">
                  <div v-for="production in store.slr1Result.productionList" :key="production.index"
                    class="production-item">
                    <span class="production-number">{{ production.index }}:</span>
                    <strong class="production-left">{{ production.leftSide }}</strong>
                    <span class="production-arrow">→</span>
                    <span class="production-right">
                      <span v-for="(symbol, idx) in production.rightSide" :key="idx" class="right-symbol">
                        {{ symbol }}
                      </span>
                    </span>
                  </div>
                </div>
                <!-- 备用显示（使用修复后的flattenedProductions） -->
                <div v-else-if="store.slr1Result?.productions && Object.keys(store.slr1Result.productions).length > 0"
                  class="production-list">
                  <div v-for="(production, index) in flattenedProductions" :key="`production-${index}`"
                    class="production-item">
                    <span class="production-number">{{ index }}:</span>
                    <strong class="production-left">{{ production.left }}</strong>
                    <span class="production-arrow">→</span>
                    <span class="production-right">
                      <span v-for="(symbol, idx) in production.right" :key="idx" class="right-symbol">
                        {{ symbol }}
                      </span>
                    </span>
                  </div>
                </div>
                <!-- 无数据提示 -->
                <div v-else class="no-data">
                  <p>暂无产生式数据</p>
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
const exampleGrammar = `S' -> S
S -> E
E -> E + T | T
T -> T * F | F
F -> ( E ) | id`

const exampleInput = 'id + id * id'

// 计算属性
const canParse = computed(() => {
  return grammarInput.value.trim() && inputString.value.trim()
})

// 扁平化产生式，确保序号连续
const flattenedProductions = computed(() => {
  // 优先使用 productionList，如果可用的话
  if (store.slr1Result?.productionList && store.slr1Result.productionList.length > 0) {
    return store.slr1Result.productionList.map(prod => ({
      left: prod.leftSide,
      right: prod.rightSide
    }))
  }

  // 备用方案：使用旧的 productions 格式
  if (!store.slr1Result?.productions) return []

  const flattened: { left: string; right: string[] }[] = []

  // 遍历所有左部符号
  for (const [left, rightsList] of Object.entries(store.slr1Result.productions)) {
    // 遍历该左部符号的所有右部
    for (const rights of rightsList) {
      flattened.push({
        left,
        right: rights
      })
    }
  }

  return flattened
})

// 从 parseTable 中提取终结符
const getTerminals = () => {
  if (!store.slr1Result?.parseTable?.headers) return []
  // 通常前几个是终结符，这里需要根据实际情况调整
  return store.slr1Result.parseTable.headers.slice(1).filter(header =>
    header === '#' || header === 'id' || header === '+' || header === '*' || header === '(' || header === ')'
  )
}

// 从 parseTable 中提取非终结符
const getNonterminals = () => {
  if (!store.slr1Result?.parseTable?.headers) return []
  // 非终结符通常是大写字母开头，且不包含 '
  return store.slr1Result.parseTable.headers.slice(1).filter(header =>
    /^[A-Z]/.test(header) && !header.includes("'")
  )
}

// 获取动作的CSS类
const getActionClass = (action: string) => {
  if (!action) return ''
  if (action === 'accept') return 'accept-action'
  if (action.startsWith('shift')) return 'shift-action'
  if (action.startsWith('reduce')) return 'reduce-action'
  if (action.includes('error') || action.includes('错误')) return 'error-action'
  return ''
}

// 获取动作单元格的CSS类
const getActionCellClass = (action: string) => {
  if (!action) return ''
  if (action === 'acc') return 'accept-cell'
  if (action.startsWith('s')) return 'shift-cell'
  if (action.startsWith('r')) return 'reduce-cell'
  return ''
}

// 处理解析
const handleParse = async () => {
  try {
    await store.parseSLR1(grammarInput.value, inputString.value)
  } catch (error) {
    console.error('SLR1 parsing failed:', error)
  }
}

// 处理清空
const handleClear = () => {
  grammarInput.value = ''
  inputString.value = ''
  store.clearSLR1Result()
  store.clearError()
}

// 加载示例
const loadExample = () => {
  grammarInput.value = exampleGrammar
  inputString.value = exampleInput
}

// 组件挂载时检查连接
onMounted(() => {
  store.checkConnection()
  loadExample()
})
</script>

<style scoped>
.slr1-view {
  min-height: 100vh;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  padding: 2rem 0;
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
  background: white;
  color: #2c3e50;
  border: 2px solid #e1e8ed;
  padding: 0.75rem 2rem;
  border-radius: 8px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.2s ease;
}

.btn-secondary:hover:not(:disabled) {
  border-color: #3498db;
  color: #3498db;
}

.btn-small {
  background: #95a5a6;
  color: white;
  border: none;
  padding: 0.5rem 1rem;
  border-radius: 4px;
  font-size: 0.8rem;
  cursor: pointer;
  transition: background 0.2s ease;
}

.btn-small:hover {
  background: #7f8c8d;
}

.loading-spinner {
  width: 16px;
  height: 16px;
  border: 2px solid #ffffff;
  border-top: 2px solid transparent;
  border-radius: 50%;
  animation: spin 1s linear infinite;
}

@keyframes spin {
  to {
    transform: rotate(360deg);
  }
}

.result-section {
  background: #f8f9fa;
  border-radius: 12px;
  padding: 2rem;
  margin-bottom: 2rem;
}

.error-message {
  background: #fee;
  border-left: 4px solid #e74c3c;
  padding: 1rem;
  border-radius: 8px;
  margin-bottom: 1rem;
}

.error-message h3 {
  margin: 0 0 0.5rem 0;
  color: #e74c3c;
}

.error-message p {
  margin: 0 0 1rem 0;
  color: #2c3e50;
}

.result-content {
  background: white;
  border-radius: 8px;
  padding: 2rem;
}

.result-header {
  margin-bottom: 2rem;
  padding: 1rem;
  background: #f8f9fa;
  border-radius: 8px;
  text-align: center;
}

.result-header h3 {
  margin: 0 0 0.5rem 0;
  font-size: 1.5rem;
}

.result-header p {
  margin: 0;
  color: #6c757d;
}

.tab-navigation {
  display: flex;
  border-bottom: 3px solid #e1e8ed;
  margin-bottom: 2rem;
  overflow-x: auto;
}

.tab-btn {
  background: none;
  border: none;
  padding: 1rem 1.5rem;
  font-weight: 600;
  color: #6c757d;
  cursor: pointer;
  border-bottom: 3px solid transparent;
  transition: all 0.3s ease;
  white-space: nowrap;
}

.tab-btn:hover {
  color: #3498db;
}

.tab-btn.active {
  color: #3498db;
  border-bottom-color: #3498db;
}

.tab-content {
  min-height: 400px;
}

.parse-steps-table h4,
.parse-table h4,
.automaton-diagram h4,
.sets-section h4,
.productions h4 {
  margin: 0 0 1rem 0;
  color: #2c3e50;
}

.table-container {
  overflow-x: auto;
  background: white;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
}

.steps-table {
  width: 100%;
  border-collapse: collapse;
  font-family: 'Fira Code', monospace;
  font-size: 0.85rem;
}

.steps-table th,
.steps-table td {
  padding: 0.75rem;
  text-align: center;
  border: 1px solid #e1e8ed;
}

.steps-table th {
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

.error-step {
  background-color: #fee !important;
}

.accept-step {
  background-color: #eef !important;
}

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
  color: #d97706;
}

.table-wrapper {
  overflow-x: auto;
  max-width: 100%;
}

.analysis-table {
  width: 100%;
  border-collapse: collapse;
  font-family: 'Fira Code', monospace;
  font-size: 0.8rem;
  min-width: 600px;
}

.analysis-table th,
.analysis-table td {
  padding: 0.5rem;
  text-align: center;
  border: 1px solid #e1e8ed;
}

.analysis-table th {
  background: #f8f9fa;
  font-weight: 600;
  color: #2c3e50;
}

.state-header {
  background: #3498db !important;
  color: white !important;
}

.action-header {
  background: #e8f4fd !important;
  color: #2c3e50 !important;
}

.goto-header {
  background: #f0f8f0 !important;
  color: #2c3e50 !important;
}

.state-cell {
  background: #f0f8ff;
  font-weight: 600;
}

.accept-cell {
  background: #d4edda;
  color: #155724;
  font-weight: 600;
}

.shift-cell {
  background: #cce5ff;
  color: #004085;
}

.reduce-cell {
  background: #fff3cd;
  color: #856404;
}

.goto-cell {
  background: #f8f9fa;
  color: #6c757d;
}

.svg-container {
  text-align: center;
  background: #f8f9fa;
  padding: 2rem;
  border-radius: 8px;
  overflow-x: auto;
}

.svg-container :deep(svg) {
  max-width: 100%;
  height: auto;
}

.no-diagram {
  text-align: center;
  color: #6c757d;
  padding: 3rem;
  background: #f8f9fa;
  border-radius: 8px;
}

.no-diagram .hint {
  font-size: 0.9rem;
  margin-top: 0.5rem;
}

.sets-container {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 2rem;
}

.first-sets,
.follow-sets {
  background: #f8f9fa;
  padding: 1.5rem;
  border-radius: 8px;
}

.set-list {
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.set-item {
  padding: 0.75rem;
  background: white;
  border-radius: 6px;
  border-left: 4px solid #3498db;
}

.set-symbols {
  color: #6c757d;
  font-family: 'Fira Code', monospace;
  margin-left: 0.5rem;
}

.production-list {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
}

.production-item {
  padding: 0.75rem;
  background: #f8f9fa;
  border-radius: 6px;
  border-left: 4px solid #3498db;
  font-family: 'Fira Code', monospace;
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.production-number {
  color: #6c757d;
  font-weight: 600;
  min-width: 2rem;
}

.production-left {
  color: #2c3e50;
  font-weight: 600;
}

.production-arrow {
  color: #3498db;
  font-weight: 600;
}

.production-right {
  color: #6c757d;
}

.right-symbol {
  margin-right: 0.25rem;
}

.connection-status {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1rem;
  background: #f8f9fa;
  border-radius: 8px;
  margin-top: 2rem;
}

.status-indicator {
  font-size: 0.9rem;
  font-weight: 600;
}

.connected {
  color: #27ae60;
}

.disconnected {
  color: #e74c3c;
}

@media (max-width: 768px) {
  .slr1-view {
    padding: 1rem 0;
  }

  .view-header {
    padding: 2rem 1rem;
  }

  .view-header h1 {
    font-size: 2rem;
  }

  .main-content {
    padding: 1rem;
  }

  .actions {
    flex-direction: column;
  }

  .tab-navigation {
    flex-wrap: wrap;
  }

  .sets-container {
    grid-template-columns: 1fr;
  }

  .connection-status {
    flex-direction: column;
    gap: 1rem;
  }
}
</style>
