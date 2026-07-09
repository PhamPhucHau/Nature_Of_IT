import { useState, useCallback } from 'react'
import * as api from '../api'

/**
 * Custom hook for chatbot logic
 * 
 * Manages:
 * - Conversations (create, select, delete)
 * - Messages (send, receive, history)
 * - Loading and error states
 * - Model selection
 */
export const useChatbot = () => {
  const [conversations, setConversations] = useState([])
  const [currentConversationId, setCurrentConversationId] = useState(null)
  const [messages, setMessages] = useState([])
  const [selectedModel, setSelectedModel] = useState('llama2:13b-chat')
  const [modelParameters, setModelParameters] = useState({
    temperature: 0.7,
    topP: 0.9,
    topK: 40,
    maxTokens: 2048,
    minTokens: 0,
    repetitionPenalty: 1.1,
    frequencyPenalty: 0.0,
    presencePenalty: 0.0,
    doSample: true,
    seed: null
  })
  const [isLoading, setIsLoading] = useState(false)
  const [error, setError] = useState(null)

  // Load all conversations
  const loadConversations = useCallback(async () => {
    try {
      setError(null)
      const data = await api.getAllConversations()
      // Ensure data is always an array
      const conversationsArray = Array.isArray(data) ? data : []
      console.log('[DEBUG] Loaded conversations:', conversationsArray)
      setConversations(conversationsArray)
    } catch (err) {
      setError(`Failed to load conversations: ${err.message}`)
      console.error('Error loading conversations:', err)
      setConversations([]) // Set empty array on error
    }
  }, [])

  // Create new conversation
  const newConversation = useCallback(async (model) => {
    try {
      setError(null)
      setIsLoading(true)
      const conversation = await api.createConversation(model)
      
      setConversations(prev => [conversation, ...prev])
      setCurrentConversationId(conversation.id)
      setMessages([])
      setSelectedModel(model)
      
      return conversation.id
    } catch (err) {
      setError(`Failed to create conversation: ${err.message}`)
      console.error('Error creating conversation:', err)
    } finally {
      setIsLoading(false)
    }
  }, [])

  // Select conversation
  const selectConversation = useCallback(async (conversationId, model) => {
    try {
      setError(null)
      setCurrentConversationId(conversationId)
      setSelectedModel(model)
      
      const history = await api.getConversationHistory(conversationId)
      setMessages(history)
    } catch (err) {
      setError(`Failed to load conversation: ${err.message}`)
      console.error('Error loading conversation:', err)
    }
  }, [])

  // Send message
  const sendMessage = useCallback(async (messageText) => {
    if (!currentConversationId) {
      setError('No conversation selected')
      return
    }

    if (!messageText.trim()) {
      return
    }

    try {
      setError(null)
      setIsLoading(true)

      // Add user message to UI immediately
      const userMessage = {
        role: 'USER',
        content: messageText,
        createdAt: new Date().toISOString()
      }
      setMessages(prev => [...prev, userMessage])

      // Send to backend
      const response = await api.sendMessage(
        currentConversationId,
        messageText,
        selectedModel,
        modelParameters
      )

      // Add assistant message
      const assistantMessage = {
        role: 'ASSISTANT',
        content: response.content,
        processingTimeMs: response.processingTimeMs,
        createdAt: new Date().toISOString()
      }
      setMessages(prev => [...prev, assistantMessage])

      // Update conversations list
      await loadConversations()

    } catch (err) {
      setError(`Failed to send message: ${err.message}`)
      console.error('Error sending message:', err)
      
      // Remove the user message we added
      setMessages(prev => prev.slice(0, -1))
    } finally {
      setIsLoading(false)
    }
  }, [currentConversationId, selectedModel, loadConversations])

  // Delete conversation
  const deleteConversation = useCallback(async (conversationId) => {
    try {
      setError(null)
      await api.deleteConversation(conversationId)
      
      setConversations(prev => 
        prev.filter(c => c.id !== conversationId)
      )
      
      if (currentConversationId === conversationId) {
        setCurrentConversationId(null)
        setMessages([])
      }
    } catch (err) {
      setError(`Failed to delete conversation: ${err.message}`)
      console.error('Error deleting conversation:', err)
    }
  }, [currentConversationId])

  return {
    conversations,
    currentConversationId,
    messages,
    selectedModel,
    modelParameters,
    isLoading,
    error,
    loadConversations,
    newConversation,
    selectConversation,
    sendMessage,
    deleteConversation,
    setError,
    setSelectedModel,
    setModelParameters
  }
}
