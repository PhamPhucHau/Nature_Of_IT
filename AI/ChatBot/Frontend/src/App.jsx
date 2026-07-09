import React, { useEffect } from 'react'
import ChatBox from './components/ChatBox'
import ConversationList from './components/ConversationList'
import { useChatbot } from './hooks/useChatbot'
import './styles/App.css'

/**
 * Main App component
 * 
 * Orchestrates:
 * - Conversation management
 * - Message handling
 * - Model selection
 * - Error handling
 */
function App() {
  const chatbot = useChatbot()

  // Load conversations on mount
  useEffect(() => {
    chatbot.loadConversations()
  }, [chatbot.loadConversations])

  const handleNewConversation = async () => {
    const conversationId = await chatbot.newConversation(chatbot.selectedModel)
    if (conversationId) {
      await chatbot.selectConversation(conversationId, chatbot.selectedModel)
    }
  }

  const handleSelectConversation = async (conversationId, model) => {
    await chatbot.selectConversation(conversationId, model)
  }

  const handleDeleteConversation = async (conversationId) => {
    await chatbot.deleteConversation(conversationId)
  }

  const handleModelChange = (newModel) => {
    // Only allow model change if we have a conversation and are not loading
    if (chatbot.currentConversationId && !chatbot.isLoading) {
      chatbot.setSelectedModel(newModel)
    }
  }

  return (
    <div className="app">
      <div className="app-sidebar">
        <ConversationList
          conversations={chatbot.conversations}
          currentConversationId={chatbot.currentConversationId}
          onSelectConversation={handleSelectConversation}
          onDeleteConversation={handleDeleteConversation}
          onNewConversation={handleNewConversation}
          isLoading={chatbot.isLoading}
        />
      </div>

      <div className="app-main">
        <ChatBox
          messages={chatbot.messages}
          onSendMessage={chatbot.sendMessage}
          isLoading={chatbot.isLoading}
          selectedModel={chatbot.selectedModel}
          onModelChange={handleModelChange}
          modelParameters={chatbot.modelParameters}
          onParametersChange={chatbot.setModelParameters}
          error={chatbot.error}
          onNewConversation={handleNewConversation}
          currentConversationId={chatbot.currentConversationId}
        />
      </div>
    </div>
  )
}

export default App
