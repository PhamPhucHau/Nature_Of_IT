import React from 'react'
import '../styles/ConversationList.css'

/**
 * ConversationList component
 * 
 * Displays all saved conversations and allows:
 * - Selecting a conversation
 * - Deleting a conversation
 * - Creating a new conversation
 */
const ConversationList = ({
  conversations,
  currentConversationId,
  onSelectConversation,
  onDeleteConversation,
  onNewConversation,
  isLoading
}) => {
  const formatDate = (isoString) => {
    if (!isoString) return ''
    const date = new Date(isoString)
    return date.toLocaleDateString('en-US', {
      month: 'short',
      day: 'numeric',
      hour: '2-digit',
      minute: '2-digit'
    })
  }

  return (
    <div className="conversation-list-container">
      <div className="conversation-list-header">
        <h2>💬 Conversations</h2>
        <button
          className="btn-new-conv"
          onClick={onNewConversation}
          disabled={isLoading}
          title="Start a new conversation"
        >
          ➕ New
        </button>
      </div>

      <div className="conversation-list">
        {!Array.isArray(conversations) || conversations.length === 0 ? (
          <div className="empty-conversations">
            <p>No conversations yet</p>
            <p style={{ fontSize: '0.85em', color: '#999' }}>
              Click "➕ New" to create one
            </p>
          </div>
        ) : (
          conversations.map(conv => (
            <div
              key={conv.id}
              className={`conversation-item ${
                conv.id === currentConversationId ? 'active' : ''
              }`}
              onClick={() => onSelectConversation(conv.id, conv.model)}
            >
              <div className="conv-info">
                <div className="conv-title">{conv.title}</div>
                <div className="conv-meta">
                  <span className="conv-model">{conv.model}</span>
                  <span className="conv-date">
                    {formatDate(conv.updatedAt)}
                  </span>
                </div>
              </div>
              <button
                className="btn-delete-conv"
                onClick={(e) => {
                  e.stopPropagation()
                  if (confirm('Delete this conversation?')) {
                    onDeleteConversation(conv.id)
                  }
                }}
                title="Delete conversation"
              >
                🗑️
              </button>
            </div>
          ))
        )}
      </div>
    </div>
  )
}

export default ConversationList
