import React from 'react'
import '../styles/MessageList.css'

/**
 * MessageList component
 * 
 * Displays all messages in the current conversation
 * Each message shows:
 * - Role (User / Assistant)
 * - Content
 * - Timestamp
 * - Request ID (for tracing)
 * - Processing time (for assistant responses)
 */
const MessageList = ({ messages }) => {
  const formatTime = (isoString) => {
    if (!isoString) return ''
    const date = new Date(isoString)
    return date.toLocaleTimeString('en-US', { 
      hour: '2-digit', 
      minute: '2-digit',
      second: '2-digit'
    })
  }

  const formatProcessingTime = (ms) => {
    if (!ms) return ''
    if (ms < 1000) return `${ms}ms`
    return `${(ms / 1000).toFixed(2)}s`
  }

  return (
    <div className="message-list">
      {messages.length === 0 ? (
        <div className="empty-state">
          <p>No messages yet</p>
          <p style={{ fontSize: '0.9em', color: '#999' }}>Send a message to start</p>
        </div>
      ) : (
        messages.map((msg, index) => (
          <div
            key={index}
            className={`message message-${msg.role.toLowerCase()}`}
          >
            <div className="message-header">
              <span className="message-role">
                {msg.role === 'USER' ? '👤 You' : '🤖 Assistant'}
              </span>
              <span className="message-time">
                {formatTime(msg.createdAt)}
              </span>
            </div>

            <div className="message-content">
              {msg.content}
            </div>

            <div className="message-metadata">
              {msg.processingTimeMs && (
                <span className="metadata-item">
                  ⏱️ {formatProcessingTime(msg.processingTimeMs)}
                </span>
              )}
              {msg.requestId && (
                <span className="metadata-item" title="Request ID for tracing">
                  🔗 {msg.requestId}
                </span>
              )}
            </div>
          </div>
        ))
      )}
    </div>
  )
}

export default MessageList
