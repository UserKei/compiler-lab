import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { apiService, type LR0ParseResponse, type SLR1ParseResponse, type RegexResponse } from '../services/api'

export const useCompilerStore = defineStore('compiler', () => {
  // 状态
  const isLoading = ref(false)
  const error = ref<string | null>(null)
  const isConnected = ref(false)

  // LR0 相关状态
  const lr0Result = ref<LR0ParseResponse | null>(null)
  const lr0Grammar = ref('')
  const lr0Input = ref('')

  // SLR1 相关状态
  const slr1Result = ref<SLR1ParseResponse | null>(null)
  const slr1Grammar = ref('')
  const slr1Input = ref('')

  // 正则表达式相关状态
  const regexResult = ref<RegexResponse | null>(null)
  const regex = ref('')
  const regexInput = ref('')

  // 计算属性
  const hasLR0Result = computed(() => lr0Result.value !== null)
  const hasSLR1Result = computed(() => slr1Result.value !== null)
  const hasRegexResult = computed(() => regexResult.value !== null)
  const isLR0Success = computed(() => lr0Result.value?.success || false)
  const isSLR1Success = computed(() => slr1Result.value?.success || false)
  const isRegexSuccess = computed(() => regexResult.value?.success || false)

  // 操作
  const clearError = () => {
    error.value = null
  }

  const setLoading = (loading: boolean) => {
    isLoading.value = loading
    if (loading) {
      error.value = null
    }
  }

  // 检查服务器连接
  const checkConnection = async () => {
    try {
      setLoading(true)
      await apiService.checkHealth()
      isConnected.value = true
      return true
    } catch (err) {
      isConnected.value = false
      error.value = err instanceof Error ? err.message : '连接服务器失败'
      return false
    } finally {
      setLoading(false)
    }
  }

  // LR0 语法分析
  const parseLR0 = async (grammar: string, input: string) => {
    try {
      setLoading(true)
      lr0Grammar.value = grammar
      lr0Input.value = input

      const result = await apiService.parseLR0({ grammar, input })
      lr0Result.value = result
      return result
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'LR0分析失败'
      lr0Result.value = null
      throw err
    } finally {
      setLoading(false)
    }
  }

  // SLR1 语法分析
  const parseSLR1 = async (grammar: string, input: string) => {
    try {
      setLoading(true)
      slr1Grammar.value = grammar
      slr1Input.value = input

      const result = await apiService.parseSLR1({ grammar, input })
      slr1Result.value = result
      return result
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'SLR1分析失败'
      slr1Result.value = null
      throw err
    } finally {
      setLoading(false)
    }
  }

  // 构建正则表达式自动机
  const buildRegex = async (regexPattern: string) => {
    try {
      setLoading(true)
      regex.value = regexPattern

      const result = await apiService.buildRegex({ regex: regexPattern })
      regexResult.value = result
      return result
    } catch (err) {
      error.value = err instanceof Error ? err.message : '正则表达式构建失败'
      regexResult.value = null
      throw err
    } finally {
      setLoading(false)
    }
  }

  // 正则表达式匹配
  const matchRegex = async (regexPattern: string, input: string) => {
    try {
      setLoading(true)
      regex.value = regexPattern
      regexInput.value = input

      const result = await apiService.matchRegex({ regex: regexPattern, input })
      regexResult.value = result
      return result
    } catch (err) {
      error.value = err instanceof Error ? err.message : '正则表达式匹配失败'
      regexResult.value = null
      throw err
    } finally {
      setLoading(false)
    }
  }

  // 清空结果
  const clearLR0Result = () => {
    lr0Result.value = null
    lr0Grammar.value = ''
    lr0Input.value = ''
  }

  const clearSLR1Result = () => {
    slr1Result.value = null
    slr1Grammar.value = ''
    slr1Input.value = ''
  }

  const clearRegexResult = () => {
    regexResult.value = null
    regex.value = ''
    regexInput.value = ''
  }

  const clearAllResults = () => {
    clearLR0Result()
    clearSLR1Result()
    clearRegexResult()
    clearError()
  }

  return {
    // 状态
    isLoading,
    error,
    isConnected,
    lr0Result,
    lr0Grammar,
    lr0Input,
    slr1Result,
    slr1Grammar,
    slr1Input,
    regexResult,
    regex,
    regexInput,

    // 计算属性
    hasLR0Result,
    hasSLR1Result,
    hasRegexResult,
    isLR0Success,
    isSLR1Success,
    isRegexSuccess,

    // 操作
    clearError,
    checkConnection,
    parseLR0,
    parseSLR1,
    buildRegex,
    matchRegex,
    clearLR0Result,
    clearSLR1Result,
    clearRegexResult,
    clearAllResults,
  }
})
