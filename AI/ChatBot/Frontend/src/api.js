import axios from 'axios'
import { v4 as uuidv4 } from 'uuid'

/**
 * API service with automatic request ID injection
 * 
 * Every request includes X-Request-ID header with format: req_xxxxxxxx
 * This enables end-to-end tracing through the entire stack
 */

// Create axios instance
const api = axios.create({
  baseURL: '/api',
  timeout: 300000,
  headers: {
    'Content-Type': 'application/json'
  }
})

/**
 * Request interceptor - add request ID to every request
 */
api.interceptors.request.use((config) => {
  const requestId = `req_${uuidv4().substring(0, 8)}`
  config.headers['X-Request-ID'] = requestId
  
  console.log(`[${requestId}] Request: ${config.method.toUpperCase()} ${config.url}`)
  
  return config
}, (error) => {
  console.error('Request error:', error)
  return Promise.reject(error)
})

/**
 * Response interceptor - log responses and extract request ID
 */
api.interceptors.response.use((response) => {
  const requestId = response.headers['x-request-id'] || 'unknown'
  console.log(`[${requestId}] Response: ${response.status}`)
  return response
}, (error) => {
  const requestId = error.response?.headers['x-request-id'] || 'unknown'
  console.error(`[${requestId}] Error:`, error.message)
  return Promise.reject(error)
})

// ============================================================================
// Chat API Functions
// ============================================================================

/**
 * Send a message to the chatbot
 */
export const sendMessage = async (conversationId, message, model, parameters = null) => {
  const payload = {
    conversationId,
    message,
    model
  }
  
  // Include parameters if provided
  if (parameters) {
    payload.parameters = parameters
  }
  
  const response = await api.post('/chat/send', payload)
  return response.data
}

/**
 * Create a new conversation
 */
export const createConversation = async (model = 'llama2') => {
  const response = await api.post('/chat/conversations', { model })
  return response.data
}

/**
 * Get conversation history
 */
export const getConversationHistory = async (conversationId) => {
  const response = await api.get(`/chat/history/${conversationId}`)
  return response.data
}

/**
 * Delete a conversation
 */
export const deleteConversation = async (conversationId) => {
  await api.delete(`/chat/history/${conversationId}`)
}

/**
 * Get all conversations
 */
export const getAllConversations = async () => {
  const response = await api.get('/chat/conversations')
  return response.data
}

/**
 * Health check
 */
export const healthCheck = async () => {
  const response = await api.get('/chat/health')
  return response.data
}

export default api
