import React, { useState, useEffect, useRef } from 'react'
import MessageList from './MessageList'
import ModelSelector from './ModelSelector'
import ParameterSettings from './ParameterSettings'
import '../styles/ChatBox.css'

/**
 * Main ChatBox component
 * 
 * Combines:
 * - Message display (MessageList)
 * - Input field
 * - Model selector
 * - Parameter settings
 * - Conversation management
 */
const ChatBox = ({
  messages,
  onSendMessage,
  isLoading,
  selectedModel,
  onModelChange,
  modelParameters,
  onParametersChange,
  error,
  onNewConversation,
  currentConversationId
}) => {
  const [inputValue, setInputValue] = useState('')
  const messagesEndRef = useRef(null)

  // Auto-scroll to bottom
  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' })
  }, [messages])

  const handleSendMessage = (e) => {
    e.preventDefault()
    if (inputValue.trim() && !isLoading) {
      onSendMessage(inputValue)
      setInputValue('')
    }
  }

  return (
    <div className="chatbox-container">
      <div className="chatbox-header">
        <h1>💬 ChatBot</h1>
        <div className="header-controls">
          <ModelSelector 
            selectedModel={selectedModel} 
            onModelChange={onModelChange}
            disabled={!currentConversationId}
          />
          <button 
            className="btn-new-conversation"
            onClick={onNewConversation}
            disabled={isLoading}
            title="Start a new conversation"
          >
            ➕ New
          </button>
        </div>
      </div>

      <ParameterSettings 
        parameters={modelParameters}
        onParametersChange={onParametersChange}
        disabled={!currentConversationId || isLoading}
      />

      {error && (
        <div className="error-banner">
          ⚠️ {error}
        </div>
      )}

      {!currentConversationId && (
        <div className="welcome-banner">
          <h2>Welcome to ChatBot!</h2>
          <p>Click "➕ New" to start a conversation</p>
          <p style={{ fontSize: '0.9em', color: '#666' }}>
            Powered by Ollama • Real-time Local LLM • Request Tracing
          </p>
          <button 
            className="btn-quick-new"
            onClick={onNewConversation}
            disabled={isLoading}
            style={{
              marginTop: '20px',
              padding: '12px 24px',
              fontSize: '16px',
              backgroundColor: '#7c3aed',
              color: 'white',
              border: 'none',
              borderRadius: '8px',
              cursor: isLoading ? 'not-allowed' : 'pointer',
              opacity: isLoading ? 0.6 : 1
            }}
          >
            ➕ Start New Conversation
          </button>
        </div>
      )}

      <div className="messages-container">
        <MessageList messages={messages} />
        <div ref={messagesEndRef} />
      </div>

      <form className="message-input-form" onSubmit={handleSendMessage}>
        <div className="input-wrapper">
          <input
            type="text"
            value={inputValue}
            onChange={(e) => setInputValue(e.target.value)}
            placeholder={
              currentConversationId 
                ? 'Type your message...' 
                : 'Create a conversation to start chatting'
            }
            disabled={!currentConversationId || isLoading}
            className="message-input"
            autoFocus
          />
          <button
            type="submit"
            disabled={!inputValue.trim() || !currentConversationId || isLoading}
            className="btn-send"
            title="Send message"
          >
            {isLoading ? '⏳' : '📤'}
          </button>
        </div>
        {isLoading && (
          <div className="loading-indicator">
            ⏳ AI is thinking...
          </div>
        )}
      </form>
    </div>
  )
}

export default ChatBox
