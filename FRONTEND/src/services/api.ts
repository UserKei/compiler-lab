// API服务模块
const API_BASE_URL = '/api'

export interface LR0ParseRequest {
  grammar: string
  input: string
}

export interface LR0ParseResponse {
  success: boolean
  message: string
  parseSteps: string[]
  productions: Record<string, string[]>
}

export interface RegexBuildRequest {
  regex: string
}

export interface RegexMatchRequest {
  regex: string
  input: string
}

export interface RegexResponse {
  success: boolean
  message: string
  nfaDescription: string
  dfaDescription: string
  matchResult?: boolean
}

export interface HealthResponse {
  status: string
  message: string
  timestamp: number
}

class ApiService {
  private async request<T>(endpoint: string, options: RequestInit = {}): Promise<T> {
    const url = `${API_BASE_URL}${endpoint}`

    const defaultOptions: RequestInit = {
      headers: {
        'Content-Type': 'application/json',
      },
    }

    const response = await fetch(url, {
      ...defaultOptions,
      ...options,
      headers: {
        ...defaultOptions.headers,
        ...options.headers,
      },
    })

    if (!response.ok) {
      const errorData = await response.json().catch(() => ({ error: 'Network error' }))
      throw new Error(errorData.error || `HTTP ${response.status}`)
    }

    return response.json()
  }

  // 健康检查
  async checkHealth(): Promise<HealthResponse> {
    return this.request<HealthResponse>('/health', {
      method: 'GET',
    })
  }

  // LR0语法分析
  async parseLR0(data: LR0ParseRequest): Promise<LR0ParseResponse> {
    return this.request<LR0ParseResponse>('/lr0/parse', {
      method: 'POST',
      body: JSON.stringify(data),
    })
  }

  // 构建正则表达式自动机
  async buildRegex(data: RegexBuildRequest): Promise<RegexResponse> {
    return this.request<RegexResponse>('/regex/build', {
      method: 'POST',
      body: JSON.stringify(data),
    })
  }

  // 正则表达式匹配
  async matchRegex(data: RegexMatchRequest): Promise<RegexResponse> {
    return this.request<RegexResponse>('/regex/match', {
      method: 'POST',
      body: JSON.stringify(data),
    })
  }

  // 上传语法文件
  async uploadGrammar(file: File): Promise<any> {
    const formData = new FormData()
    formData.append('grammar', file)

    return this.request('/grammar/upload', {
      method: 'POST',
      body: formData,
      headers: {}, // 让浏览器自动设置Content-Type
    })
  }
}

export const apiService = new ApiService()
