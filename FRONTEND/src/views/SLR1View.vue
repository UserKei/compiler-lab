<template>
  <div class="slr1-view">
    <div class="container">
      <h1 class="title">SLR(1) 语法分析器</h1>
      <p class="subtitle">
        SLR(1) 是 LR(0) 的改进版本，使用 FOLLOW 集合来解决 LR(0) 的冲突问题
      </p>

      <!-- 输入区域 -->
      <div class="input-section">
        <div class="input-group">
          <label for="grammar" class="input-label">语法规则</label>
          <textarea id="grammar" v-model="grammarInput" class="grammar-textarea" placeholder="请输入语法规则，格式：A -> B C | D"
            rows="8"></textarea>
          <div class="input-hint">
            每行一个产生式，使用 " -> " 分隔左部和右部，使用 " | " 分隔多个选择
          </div>
        </div>

        <div class="input-group">
          <label for="input" class="input-label">输入字符串</label>
          <input id="input" v-model="inputString" type="text" class="input-field" placeholder="请输入要分析的字符串" />
        </div>

        <div class="button-group">
          <button @click="analyze" :disabled="isLoading || !grammarInput.trim() || !inputString.trim()"
            class="analyze-button">
            <span v-if="isLoading" class="loading-spinner"></span>
            {{ isLoading ? '分析中...' : '开始分析' }}
          </button>
          <button @click="clearResults" class="clear-button">清空结果</button>
        </div>
      </div>

      <!-- 错误信息 -->
      <div v-if="error" class="error-message">
        <strong>错误：</strong>{{ error }}
      </div>

      <!-- 结果区域 -->
      <div v-if="hasSLR1Result" class="results-section">
        <!-- 分析状态 -->
        <div class="status-section">
          <div :class="[
            'status-badge',
            isSLR1Success && slr1Result?.isAccepted ? 'success' : 'error'
          ]">
            {{ isSLR1Success && slr1Result?.isAccepted ? '✅ 接受' : '❌ 拒绝' }}
          </div>
          <p class="status-message">{{ slr1Result?.message }}</p>
        </div>

        <!-- 标签页 -->
        <div class="tabs">
          <button v-for="tab in tabs" :key="tab.id" @click="activeTab = tab.id"
            :class="['tab', { active: activeTab === tab.id }]">
            {{ tab.label }}
          </button>
        </div>

        <!-- 标签页内容 -->
        <div class="tab-content">
          <!-- 分析步骤 -->
          <div v-if="activeTab === 'steps'" class="steps-section">
            <h3>分析步骤</h3>
            <div class="table-wrapper">
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
                  <tr v-for="step in slr1Result?.parseSteps" :key="step.step" :class="getStepClass(step.action)">
                    <td>{{ step.step }}</td>
                    <td class="monospace">{{ step.stateStack }}</td>
                    <td class="monospace">{{ step.symbolStack }}</td>
                    <td class="monospace">{{ step.remainingInput }}</td>
                    <td class="monospace action">{{ step.action }}</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>

          <!-- 分析表 -->
          <div v-if="activeTab === 'table'" class="table-section">
            <h3>SLR(1) 分析表</h3>
            <div class="table-wrapper">
              <table class="parse-table">
                <thead>
                  <tr>
                    <th>状态</th>
                    <th v-for="header in slr1Result?.parseTable.headers.slice(1)" :key="header">
                      {{ header }}
                    </th>
                  </tr>
                </thead>
                <tbody>
                  <tr v-for="row in slr1Result?.parseTable.rows" :key="row.state">
                    <td class="state-cell">{{ row.state }}</td>
                    <td v-for="header in slr1Result?.parseTable.headers.slice(1)" :key="header" class="action-cell">
                      <span v-if="row.actions[header] || row.gotos[header] !== undefined"
                        :class="getActionClass(row.actions[header] || row.gotos[header]?.toString())">
                        {{ row.actions[header] || row.gotos[header] }}
                      </span>
                    </td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>

          <!-- 自动机图 -->
          <div v-if="activeTab === 'automaton'" class="automaton-section">
            <h3>SLR(1) 自动机</h3>
            <div v-if="slr1Result?.svgDiagram" class="svg-container" v-html="slr1Result.svgDiagram"></div>
            <div v-else class="no-diagram">无法生成自动机图</div>
          </div>

          <!-- FIRST/FOLLOW集合 -->
          <div v-if="activeTab === 'sets'" class="sets-section">
            <div class="sets-container">
              <div class="first-sets">
                <h3>FIRST 集合</h3>
                <div class="set-list">
                  <div v-for="(symbols, nonterminal) in slr1Result?.firstSets" :key="nonterminal" class="set-item">
                    <strong>FIRST({{ nonterminal }}) =</strong>
                    <span class="set-symbols">{ {{ symbols.join(', ') }} }</span>
                  </div>
                </div>
              </div>

              <div class="follow-sets">
                <h3>FOLLOW 集合</h3>
                <div class="set-list">
                  <div v-for="(symbols, nonterminal) in slr1Result?.followSets" :key="nonterminal" class="set-item">
                    <strong>FOLLOW({{ nonterminal }}) =</strong>
                    <span class="set-symbols">{ {{ symbols.join(', ') }} }</span>
                  </div>
                </div>
              </div>
            </div>
          </div>

          <!-- 产生式 -->
          <div v-if="activeTab === 'productions'" class="productions-section">
            <h3>产生式</h3>
            <div class="productions-list">
              <div v-for="(rightSides, leftSide) in slr1Result?.productions" :key="leftSide" class="production-group">
                <div v-for="(rightSide, index) in rightSides" :key="`${leftSide}-${index}`" class="production-item">
                  <span class="production-left">{{ leftSide }}</span>
                  <span class="production-arrow">→</span>
                  <span class="production-right">{{ rightSide.join(' ') }}</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { useCompilerStore } from '../stores/compiler'

const store = useCompilerStore()

// 响应式数据
const grammarInput = ref(`S' -> S
S -> E
E -> E + T | T
T -> T * F | F
F -> ( E ) | id`)

const inputString = ref('id + id * id')
const activeTab = ref('steps')

// 标签页配置
const tabs = [
  { id: 'steps', label: '分析步骤' },
  { id: 'table', label: '分析表' },
  { id: 'automaton', label: '自动机图' },
  { id: 'sets', label: 'FIRST/FOLLOW' },
  { id: 'productions', label: '产生式' },
]

// 计算属性
const isLoading = computed(() => store.isLoading)
const error = computed(() => store.error)
const slr1Result = computed(() => store.slr1Result)
const hasSLR1Result = computed(() => store.hasSLR1Result)
const isSLR1Success = computed(() => store.isSLR1Success)

// 方法
const analyze = async () => {
  try {
    await store.parseSLR1(grammarInput.value, inputString.value)
  } catch (err) {
    console.error('SLR1 分析失败:', err)
  }
}

const clearResults = () => {
  store.clearSLR1Result()
}

const getStepClass = (action: string) => {
  if (action === 'acc') return 'accept-step'
  if (action.startsWith('s')) return 'shift-step'
  if (action.startsWith('r')) return 'reduce-step'
  if (action.includes('错误')) return 'error-step'
  return ''
}

const getActionClass = (action: string) => {
  if (!action) return ''
  if (action === 'acc') return 'accept-action'
  if (action.toString().startsWith('s')) return 'shift-action'
  if (action.toString().startsWith('r')) return 'reduce-action'
  return 'goto-action'
}
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
  padding: 0 1rem;
}

.title {
  text-align: center;
  color: white;
  font-size: 2.5rem;
  margin-bottom: 0.5rem;
  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.3);
}

.subtitle {
  text-align: center;
  color: rgba(255, 255, 255, 0.9);
  font-size: 1.1rem;
  margin-bottom: 2rem;
}

.input-section {
  background: white;
  border-radius: 12px;
  padding: 2rem;
  margin-bottom: 2rem;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
}

.input-group {
  margin-bottom: 1.5rem;
}

.input-label {
  display: block;
  font-weight: 600;
  margin-bottom: 0.5rem;
  color: #374151;
}

.grammar-textarea {
  width: 100%;
  border: 2px solid #e5e7eb;
  border-radius: 8px;
  padding: 0.75rem;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  font-size: 0.9rem;
  line-height: 1.5;
  resize: vertical;
  transition: border-color 0.2s;
}

.grammar-textarea:focus {
  outline: none;
  border-color: #667eea;
  box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
}

.input-field {
  width: 100%;
  border: 2px solid #e5e7eb;
  border-radius: 8px;
  padding: 0.75rem;
  font-size: 1rem;
  transition: border-color 0.2s;
}

.input-field:focus {
  outline: none;
  border-color: #667eea;
  box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
}

.input-hint {
  font-size: 0.85rem;
  color: #6b7280;
  margin-top: 0.25rem;
}

.button-group {
  display: flex;
  gap: 1rem;
  margin-top: 1.5rem;
}

.analyze-button,
.clear-button {
  padding: 0.75rem 1.5rem;
  border: none;
  border-radius: 8px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.2s;
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.analyze-button {
  background: #667eea;
  color: white;
}

.analyze-button:hover:not(:disabled) {
  background: #5a67d8;
  transform: translateY(-1px);
}

.analyze-button:disabled {
  background: #9ca3af;
  cursor: not-allowed;
}

.clear-button {
  background: #6b7280;
  color: white;
}

.clear-button:hover {
  background: #4b5563;
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

.error-message {
  background: #fee2e2;
  color: #dc2626;
  padding: 1rem;
  border-radius: 8px;
  margin-bottom: 2rem;
  border-left: 4px solid #dc2626;
}

.results-section {
  background: white;
  border-radius: 12px;
  padding: 2rem;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
}

.status-section {
  display: flex;
  align-items: center;
  gap: 1rem;
  margin-bottom: 2rem;
  padding: 1rem;
  background: #f9fafb;
  border-radius: 8px;
}

.status-badge {
  padding: 0.5rem 1rem;
  border-radius: 20px;
  font-weight: 600;
  font-size: 0.9rem;
}

.status-badge.success {
  background: #d1fae5;
  color: #065f46;
}

.status-badge.error {
  background: #fee2e2;
  color: #dc2626;
}

.status-message {
  margin: 0;
  color: #374151;
}

.tabs {
  display: flex;
  border-bottom: 2px solid #e5e7eb;
  margin-bottom: 2rem;
}

.tab {
  padding: 0.75rem 1.5rem;
  border: none;
  background: none;
  cursor: pointer;
  font-weight: 500;
  color: #6b7280;
  border-bottom: 2px solid transparent;
  transition: all 0.2s;
}

.tab:hover {
  color: #374151;
}

.tab.active {
  color: #667eea;
  border-bottom-color: #667eea;
}

.tab-content h3 {
  margin-bottom: 1rem;
  color: #374151;
}

.table-wrapper {
  overflow-x: auto;
  border-radius: 8px;
  border: 1px solid #e5e7eb;
}

.steps-table,
.parse-table {
  width: 100%;
  border-collapse: collapse;
  background: white;
}

.steps-table th,
.steps-table td,
.parse-table th,
.parse-table td {
  padding: 0.75rem;
  text-align: left;
  border-bottom: 1px solid #e5e7eb;
}

.steps-table th,
.parse-table th {
  background: #f9fafb;
  font-weight: 600;
  color: #374151;
}

.monospace {
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  font-size: 0.85rem;
}

.action {
  font-weight: 600;
}

.accept-step {
  background: #ecfdf5;
}

.shift-step {
  background: #eff6ff;
}

.reduce-step {
  background: #fef3c7;
}

.error-step {
  background: #fee2e2;
}

.state-cell {
  font-weight: 600;
  text-align: center;
}

.action-cell {
  text-align: center;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  font-size: 0.85rem;
}

.accept-action {
  color: #059669;
  font-weight: 600;
}

.shift-action {
  color: #2563eb;
}

.reduce-action {
  color: #d97706;
}

.goto-action {
  color: #7c3aed;
}

.svg-container {
  text-align: center;
  background: #f9fafb;
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
  color: #6b7280;
  padding: 2rem;
  background: #f9fafb;
  border-radius: 8px;
}

.sets-container {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 2rem;
}

.first-sets,
.follow-sets {
  background: #f9fafb;
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
  border-left: 4px solid #667eea;
}

.set-symbols {
  color: #6b7280;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  margin-left: 0.5rem;
}

.productions-list {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.production-group {
  background: #f9fafb;
  padding: 1rem;
  border-radius: 8px;
}

.production-item {
  padding: 0.5rem;
  background: white;
  border-radius: 6px;
  margin-bottom: 0.5rem;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
}

.production-left {
  font-weight: 600;
  color: #374151;
}

.production-arrow {
  color: #667eea;
  margin: 0 0.5rem;
  font-weight: 600;
}

.production-right {
  color: #6b7280;
}

@media (max-width: 768px) {
  .sets-container {
    grid-template-columns: 1fr;
  }

  .button-group {
    flex-direction: column;
  }

  .status-section {
    flex-direction: column;
    align-items: flex-start;
  }

  .tabs {
    overflow-x: auto;
  }
}
</style>
