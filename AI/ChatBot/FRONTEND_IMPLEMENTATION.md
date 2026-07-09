# Frontend Implementation Guide
## React ChatBot UI

---

## 🚀 Project Setup

### package.json

```json
{
  "name": "chatbot-frontend",
  "version": "1.0.0",
  "type": "module",
  "scripts": {
    "dev": "vite",
    "build": "vite build",
    "preview": "vite preview",
    "lint": "eslint src"
  },
  "dependencies": {
    "react": "^18.2.0",
    "react-dom": "^18.2.0",
    "axios": "^1.6.0",
    "react-icons": "^4.11.0",
    "uuid": "^9.0.0"
  },
  "devDependencies": {
    "@vitejs/plugin-react": "^4.0.0",
    "vite": "^4.3.0",
    "eslint": "^8.0.0"
  }
}
```

### vite.config.js

```javascript
import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

export default defineConfig({
  plugins: [react()],
  server: {
    port: 3000,
    strictPort: false,
  },
  build: {
    outDir: 'dist',
    sourcemap: true,
  }
})
```

---

## 🔌 API Service Layer

### src/services/api.js

```javascript
import axios from 'axios';
import { v4 as uuidv4 } from 'uuid';

// Create Axios instance with interceptors
const api = axios.create({
  baseURL: 'http://localhost:8080',
  timeout: 30000,
});

// Request interceptor - Add global request ID
api.interceptors.request.use(
  (config) => {
    // Generate unique request ID
    const requestId = `req_${uuidv4().substring(0, 12)}`;
    
    // Add to headers
    config.headers['X-Request-ID'] = requestId;
    config.headers['Content-Type'] = 'application/json';
    
    // Log request
    console.log(`[${requestId}] Outgoing request: ${config.method.toUpperCase()} ${config.url}`);
    console.log(`[${requestId}] Payload:`, config.data);
    
    return config;
  },
  (error) => {
    console.error('Request error:', error);
    return Promise.reject(error);
  }
);

// Response interceptor - Log responses and errors
api.interceptors.response.use(
  (response) => {
    const requestId = response.config.headers['X-Request-ID'];
    console.log(`[${requestId}] Response received: ${response.status}`);
    console.log(`[${requestId}] Data:`, response.data);
    return response;
  },
  (error) => {
    const requestId = error.config?.headers['X-Request-ID'];
    console.error(`[${requestId}] Error: ${error.message}`);
    if (error.response) {
      console.error(`[${requestId}] Status: ${error.response.status}`);
      console.error(`[${requestId}] Error data:`, error.response.data);
    }
    return Promise.reject(error);
  }
);

// API endpoints
export const chatAPI = {
  sendMessage: (message, model, conversationId) =>
    api.post('/api/chat/send', {
      message,
      model,
      conversationId,
      maxTokens: 2048,
      temperature: 0.7,
    }),

  getHistory: (conversationId) =>
    api.get(`/api/chat/history/${conversationId}`),

  clearHistory: (conversationId) =>
    api.delete(`/api/chat/history/${conversationId}`),
};

export default api;
```

---

## ⚙️ Custom Hooks

### src/hooks/useChatbot.js

```javascript
import { useState, useCallback, useRef, useEffect } from 'react';
import { chatAPI } from '../services/api';
import { v4 as uuidv4 } from 'uuid';

export const useChatbot = () => {
  const [messages, setMessages] = useState([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [conversationId, setConversationId] = useState(null);
  const [model, setModel] = useState('llama2');
  const messagesEndRef = useRef(null);

  // Initialize conversation
  useEffect(() => {
    setConversationId(`conv_${uuidv4().substring(0, 8)}`);
  }, []);

  // Auto-scroll to latest message
  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  // Send message
  const sendMessage = useCallback(
    async (userMessage) => {
      if (!userMessage.trim() || !conversationId) {
        return;
      }

      // Add user message to UI immediately
      const userMsg = {
        id: uuidv4(),
        type: 'user',
        content: userMessage,
        timestamp: new Date(),
        model,
      };

      setMessages((prev) => [...prev, userMsg]);
      setLoading(true);
      setError(null);

      try {
        console.log(`[useChatbot] Sending message with model: ${model}`);

        // Call backend API
        const response = await chatAPI.sendMessage(
          userMessage,
          model,
          conversationId
        );

        // Add bot response to UI
        const botMsg = {
          id: response.data.requestId || uuidv4(),
          type: 'bot',
          content: response.data.response,
          timestamp: new Date(),
          model: response.data.model,
          responseTime: response.data.responseTimeMs,
          tokensUsed: response.data.tokensUsed,
        };

        setMessages((prev) => [...prev, botMsg]);
        console.log(`[useChatbot] Message processed successfully`);
      } catch (err) {
        console.error('[useChatbot] Error sending message:', err);
        setError(
          err.response?.data?.message || 
          'Failed to send message. Please try again.'
        );

        // Add error message
        const errorMsg = {
          id: uuidv4(),
          type: 'error',
          content: err.message,
          timestamp: new Date(),
        };

        setMessages((prev) => [...prev, errorMsg]);
      } finally {
        setLoading(false);
      }
    },
    [conversationId, model]
  );

  // Load conversation history
  const loadHistory = useCallback(async () => {
    if (!conversationId) return;

    try {
      const response = await chatAPI.getHistory(conversationId);
      const formattedMessages = response.data.map((msg) => ({
        id: msg.requestId || uuidv4(),
        type: 'both',
        userContent: msg.userMessage,
        botContent: msg.response,
        timestamp: new Date(),
        model: msg.model,
      }));
      setMessages(formattedMessages);
    } catch (err) {
      console.error('[useChatbot] Error loading history:', err);
      setError('Failed to load chat history');
    }
  }, [conversationId]);

  // Clear conversation
  const clearConversation = useCallback(async () => {
    if (!conversationId) return;

    try {
      await chatAPI.clearHistory(conversationId);
      setMessages([]);
      setConversationId(`conv_${uuidv4().substring(0, 8)}`);
      console.log('[useChatbot] Conversation cleared');
    } catch (err) {
      console.error('[useChatbot] Error clearing conversation:', err);
      setError('Failed to clear conversation');
    }
  }, [conversationId]);

  // Change model
  const changeModel = useCallback((newModel) => {
    setModel(newModel);
    console.log(`[useChatbot] Model changed to: ${newModel}`);
  }, []);

  return {
    messages,
    loading,
    error,
    conversationId,
    model,
    messagesEndRef,
    sendMessage,
    loadHistory,
    clearConversation,
    changeModel,
  };
};
```

---

## 🎨 React Components

### src/components/ChatBox.jsx

```javascript
import React, { useState } from 'react';
import { FiSend, FiTrash2 } from 'react-icons/fi';
import { useChatbot } from '../hooks/useChatbot';
import ModelSelector from './ModelSelector';
import MessageList from './MessageList';
import '../styles/ChatBox.css';

export default function ChatBox() {
  const {
    messages,
    loading,
    error,
    model,
    messagesEndRef,
    sendMessage,
    clearConversation,
    changeModel,
  } = useChatbot();

  const [inputValue, setInputValue] = useState('');

  const handleSendMessage = (e) => {
    e.preventDefault();
    if (inputValue.trim()) {
      sendMessage(inputValue);
      setInputValue('');
    }
  };

  return (
    <div className="chatbox-container">
      <div className="chatbox-header">
        <h1>ChatBot Assistant</h1>
        <ModelSelector model={model} onModelChange={changeModel} />
        <button 
          className="clear-btn" 
          onClick={clearConversation}
          title="Clear conversation"
        >
          <FiTrash2 /> Clear
        </button>
      </div>

      <MessageList messages={messages} messagesEndRef={messagesEndRef} />

      {error && (
        <div className="error-message">
          ⚠️ {error}
        </div>
      )}

      <form className="chatbox-input" onSubmit={handleSendMessage}>
        <input
          type="text"
          value={inputValue}
          onChange={(e) => setInputValue(e.target.value)}
          placeholder="Type your message..."
          disabled={loading}
          className="message-input"
        />
        <button 
          type="submit" 
          disabled={loading || !inputValue.trim()}
          className="send-btn"
        >
          {loading ? 'Sending...' : <FiSend />}
        </button>
      </form>
    </div>
  );
}
```

### src/components/MessageList.jsx

```javascript
import React from 'react';
import { FiClock, FiZap } from 'react-icons/fi';
import '../styles/MessageList.css';

export default function MessageList({ messages, messagesEndRef }) {
  return (
    <div className="message-list">
      {messages.length === 0 ? (
        <div className="empty-state">
          <h2>Start a conversation</h2>
          <p>Select a model and type your first message to begin.</p>
        </div>
      ) : (
        <>
          {messages.map((msg) => (
            <div key={msg.id} className={`message message-${msg.type}`}>
              {msg.type === 'user' && (
                <div className="message-bubble user-message">
                  <p>{msg.content}</p>
                  <span className="message-time">
                    {msg.timestamp.toLocaleTimeString()}
                  </span>
                </div>
              )}

              {msg.type === 'bot' && (
                <div className="message-bubble bot-message">
                  <p>{msg.content}</p>
                  <div className="message-meta">
                    <span className="message-time">
                      <FiClock /> {msg.timestamp.toLocaleTimeString()}
                    </span>
                    {msg.responseTime && (
                      <span className="response-time">
                        Response: {msg.responseTime}ms
                      </span>
                    )}
                    {msg.tokensUsed && (
                      <span className="tokens">
                        <FiZap /> {msg.tokensUsed} tokens
                      </span>
                    )}
                  </div>
                </div>
              )}

              {msg.type === 'error' && (
                <div className="message-bubble error-message">
                  <p>❌ Error: {msg.content}</p>
                </div>
              )}
            </div>
          ))}
          <div ref={messagesEndRef} />
        </>
      )}
    </div>
  );
}
```

### src/components/ModelSelector.jsx

```javascript
import React from 'react';
import '../styles/ModelSelector.css';

const MODELS = [
  { id: 'llama2', name: 'Llama 2' },
  { id: 'mistral', name: 'Mistral' },
  { id: 'neural-chat', name: 'Neural Chat' },
  { id: 'dolphin-mixtral', name: 'Dolphin Mixtral' },
];

export default function ModelSelector({ model, onModelChange }) {
  return (
    <div className="model-selector">
      <label>Model:</label>
      <select 
        value={model} 
        onChange={(e) => onModelChange(e.target.value)}
        className="model-select"
      >
        {MODELS.map((m) => (
          <option key={m.id} value={m.id}>
            {m.name}
          </option>
        ))}
      </select>
    </div>
  );
}
```

---

## 🎨 Styles

### src/styles/ChatBox.css

```css
.chatbox-container {
  display: flex;
  flex-direction: column;
  height: 100vh;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

.chatbox-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px;
  background: rgba(0, 0, 0, 0.1);
  color: white;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
}

.chatbox-header h1 {
  margin: 0;
  font-size: 24px;
}

.model-selector {
  display: flex;
  align-items: center;
  gap: 10px;
}

.model-select {
  padding: 8px 12px;
  border: none;
  border-radius: 6px;
  background: white;
  color: #333;
  font-size: 14px;
  cursor: pointer;
}

.clear-btn {
  padding: 8px 16px;
  background: rgba(255, 255, 255, 0.2);
  color: white;
  border: 2px solid white;
  border-radius: 6px;
  cursor: pointer;
  display: flex;
  align-items: center;
  gap: 6px;
  transition: all 0.3s ease;
}

.clear-btn:hover {
  background: rgba(255, 255, 255, 0.3);
}

.chatbox-input {
  display: flex;
  gap: 10px;
  padding: 20px;
  background: rgba(0, 0, 0, 0.1);
}

.message-input {
  flex: 1;
  padding: 12px 16px;
  border: 2px solid white;
  border-radius: 24px;
  font-size: 14px;
  background: white;
  outline: none;
  transition: border-color 0.3s ease;
}

.message-input:focus {
  border-color: #667eea;
}

.message-input:disabled {
  background: #e0e0e0;
  cursor: not-allowed;
}

.send-btn {
  padding: 12px 24px;
  background: white;
  color: #667eea;
  border: none;
  border-radius: 24px;
  font-weight: bold;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  transition: all 0.3s ease;
}

.send-btn:hover:not(:disabled) {
  transform: scale(1.05);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2);
}

.send-btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

.error-message {
  padding: 12px 20px;
  background: rgba(255, 0, 0, 0.2);
  color: white;
  text-align: center;
  font-weight: 500;
}
```

### src/styles/MessageList.css

```css
.message-list {
  flex: 1;
  overflow-y: auto;
  padding: 20px;
  display: flex;
  flex-direction: column;
  gap: 15px;
}

.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: white;
  text-align: center;
}

.empty-state h2 {
  font-size: 28px;
  margin-bottom: 10px;
}

.empty-state p {
  font-size: 16px;
  opacity: 0.8;
}

.message {
  display: flex;
  animation: slideIn 0.3s ease;
}

@keyframes slideIn {
  from {
    opacity: 0;
    transform: translateY(10px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.message-user {
  justify-content: flex-end;
}

.message-bot {
  justify-content: flex-start;
}

.message-bubble {
  max-width: 70%;
  padding: 12px 16px;
  border-radius: 12px;
  word-wrap: break-word;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
}

.user-message {
  background: white;
  color: #333;
}

.bot-message {
  background: rgba(255, 255, 255, 0.9);
  color: #333;
}

.error-message {
  background: rgba(255, 0, 0, 0.2);
  color: #ff6b6b;
  border: 2px solid #ff6b6b;
}

.message-bubble p {
  margin: 0 0 8px 0;
  line-height: 1.5;
}

.message-time {
  display: flex;
  align-items: center;
  gap: 4px;
  font-size: 12px;
  opacity: 0.7;
}

.message-meta {
  display: flex;
  gap: 12px;
  font-size: 12px;
  opacity: 0.7;
  margin-top: 8px;
  flex-wrap: wrap;
}

.response-time,
.tokens {
  display: flex;
  align-items: center;
  gap: 4px;
}

/* Scrollbar styling */
.message-list::-webkit-scrollbar {
  width: 8px;
}

.message-list::-webkit-scrollbar-track {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 4px;
}

.message-list::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.3);
  border-radius: 4px;
}

.message-list::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.5);
}
```

### src/styles/ModelSelector.css

```css
.model-selector {
  display: flex;
  align-items: center;
  gap: 10px;
  font-weight: 500;
}

.model-selector label {
  margin: 0;
  white-space: nowrap;
}

.model-select {
  padding: 8px 12px;
  border: 2px solid rgba(255, 255, 255, 0.5);
  border-radius: 6px;
  background: rgba(255, 255, 255, 0.1);
  color: white;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.3s ease;
}

.model-select:hover {
  border-color: white;
  background: rgba(255, 255, 255, 0.2);
}

.model-select:focus {
  outline: none;
  border-color: white;
  background: rgba(255, 255, 255, 0.3);
}

.model-select option {
  background: #333;
  color: white;
}
```

---

## 📄 Main Application Files

### src/App.jsx

```javascript
import React from 'react';
import ChatBox from './components/ChatBox';
import './App.css';

function App() {
  return (
    <div className="app">
      <ChatBox />
    </div>
  );
}

export default App;
```

### src/App.css

```css
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}

.app {
  width: 100%;
  height: 100vh;
}

#root {
  width: 100%;
  height: 100%;
}
```

### src/main.jsx

```javascript
import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App.jsx'
import './styles/index.css'

ReactDOM.createRoot(document.getElementById('root')).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
)
```

### index.html

```html
<!doctype html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <link rel="icon" type="image/svg+xml" href="/vite.svg" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>ChatBot Assistant</title>
  </head>
  <body>
    <div id="root"></div>
    <script type="module" src="/src/main.jsx"></script>
  </body>
</html>
```

---

## 🚀 Running the Frontend

```bash
# Install dependencies
npm install

# Run development server
npm run dev

# Build for production
npm build

# Preview production build
npm run preview
```

Frontend will be available at `http://localhost:3000`
