# ChatBot UI - User Guide

## 🚀 Getting Started

### Step 1: Open the Application
Open your browser and navigate to:
```
http://localhost:3000
```

### Step 2: Create Your First Conversation
You'll see the welcome screen:
- Click the **"➕ Start New Conversation"** button in the center
- Or click **"➕ New"** button in the top right

**Result:** A new conversation will be created and the input field will become active.

### Step 3: Start Chatting
Once a conversation is created:
1. **Type your message** in the input field at the bottom
2. **Press Enter** or click the **📤 Send** button
3. Wait for the AI response (llama2 model)

---

## 📋 Interface Layout

### Top Section (Header)
```
[💬 ChatBot] [Model: llama2 ▼] [Fast and powerful] [➕ New]
```
- **Model Selector**: Choose which AI model to use (currently llama2)
- **➕ New Button**: Create a new conversation

### Left Sidebar (Conversations)
```
[💬 Conversations] [➕ New]
├── Conversation 1 [🗑️]
├── Conversation 2 [🗑️]
└── Conversation 3 [🗑️]
```
- **List of past conversations**: Click to select and continue
- **🗑️ Delete button**: Remove a conversation
- **➕ New button**: Create new conversation

### Main Area (Chat)
```
Welcome to ChatBot!
Click "➕ New" to start a conversation

[Message history area]

[Type your message... | 📤]
```

---

## ✨ Features & How to Use

### 1️⃣ Create Conversation
- Click **"➕ New"** button anywhere
- Conversation is created with default name: "Conversation with llama2"
- Message input field becomes **active** (enabled)

### 2️⃣ Type Message
- Click in the message input field
- Type your question or message
- **Input is only enabled AFTER creating a conversation**

### 3️⃣ Send Message
**Option A:** Press Enter
```
Type message → Press Enter → Message sends
```

**Option B:** Click Send Button
```
Type message → Click 📤 Send → Message sends
```

### 4️⃣ View Responses
- Your message appears with **USER** label
- AI response appears with **ASSISTANT** label
- Processing time shown: "Processing took 2.3s"

### 5️⃣ Switch Conversation
- Click any conversation in the **left sidebar**
- Previous messages load automatically
- Input field remains **active**

### 6️⃣ Delete Conversation
- Hover over a conversation in sidebar
- Click the **🗑️** delete button
- Confirm deletion in popup
- Conversation removed from list

### 7️⃣ Change Model
- Click the **Model selector** dropdown (shows "llama2")
- Select different model (currently only llama2 available)
- Model applies to current conversation

---

## ⚠️ Common Issues & Solutions

### Issue 1: Input Field is Grayed Out / Disabled
**Problem:** Text input field won't let you type

**Solution:** 
1. Click **"➕ New"** button to create a conversation
2. Once conversation is created, input field will be **enabled**
3. Now you can type and send messages

**Why:** The app requires a conversation context before allowing messages. This ensures every message belongs to a conversation.

---

### Issue 2: Can't Find the New Button
**Solution:** Look in TWO places:
1. **Top right corner** - "➕ New" in the header
2. **Welcome banner** - "➕ Start New Conversation" large button in center
3. **Left sidebar** - "➕ New" above conversations list

Pick any of them - they all do the same thing.

---

### Issue 3: Messages Not Sending
**Checklist:**
- ✅ Have you created a conversation? (If not, create one)
- ✅ Is the input field enabled? (It should not be grayed out)
- ✅ Did you type something? (Empty messages won't send)
- ✅ Is the AI thinking? (Wait for response indicator to finish)

---

### Issue 4: "No conversations yet" in Sidebar
**Why:** You haven't created any conversations yet

**Solution:** 
1. Click **"➕ New"** button
2. A new conversation will appear in the sidebar
3. Input field will become active

---

## 🎯 Quick Usage Flow

```
1. Open http://localhost:3000
   ↓
2. See welcome screen
   ↓
3. Click "➕ New" button
   ↓
4. Conversation created ✓
   ↓
5. Type message in input field
   ↓
6. Press Enter or click Send
   ↓
7. See AI response
   ↓
8. Continue chatting!
```

---

## 💡 Tips & Tricks

### Tip 1: Multiple Conversations
- Create multiple conversations for different topics
- Switch between them by clicking in the sidebar
- Each conversation maintains its own message history

### Tip 2: Model Selection
- Select model BEFORE or AFTER creating conversation
- Current model shown in header: "llama2"
- Model applies to that conversation only

### Tip 3: Conversation Management
- Conversations are saved automatically
- Delete old conversations with 🗑️ button
- Conversations persist when you refresh page

### Tip 4: Message History
- Click a conversation to view all past messages
- Messages automatically load from database
- Full history shows user and assistant messages

### Tip 5: Request Tracing
- Each request has a unique ID: `req_XXXXXXXX`
- View request IDs in browser console (F12)
- Used for debugging and tracking

---

## 🔧 Troubleshooting

### Frontend Won't Load
```bash
# Check if frontend is running
docker ps | grep chatbot-frontend

# If not running, restart it
docker-compose up -d frontend

# Check logs
docker logs chatbot-frontend
```

### Backend Not Responding
```bash
# Check backend health
curl http://localhost:8080/api/chat/health

# Restart backend if needed
docker-compose up -d backend
```

### Database Issues
```bash
# Connect to PostgreSQL
docker exec -it chatbot-postgres psql -U chatbot_user -d chatbot

# Check conversations table
SELECT id, title, model FROM conversations LIMIT 5;
```

---

## 📱 Browser Compatibility

**Tested on:**
- ✅ Chrome/Chromium 120+
- ✅ Firefox 121+
- ✅ Safari 17+
- ✅ Edge 120+

**Requirements:**
- JavaScript enabled
- Cookies enabled (for session)
- No ad blockers interfering

---

## 🆘 Still Not Working?

### Check These Steps:

1. **Is frontend running?**
   ```bash
   curl -s http://localhost:3000 | grep -i chatbot
   ```

2. **Is backend running?**
   ```bash
   curl http://localhost:8080/api/chat/health
   ```

3. **Check browser console** (F12)
   - Look for error messages
   - Check network tab for API calls
   - Look for `[DEBUG]` logs

4. **Try hard refresh**
   ```
   Ctrl+Shift+R (Windows/Linux)
   Cmd+Shift+R (Mac)
   ```

5. **Clear cache and cookies**
   - Go to Settings → Privacy
   - Clear cache and cookies for localhost:3000

---

## ✅ Quick Checklist

Before you start chatting, verify:
- [ ] Frontend is running at http://localhost:3000
- [ ] Backend is running at http://localhost:8080
- [ ] No errors in browser console
- [ ] You created a conversation (click ➕ New)
- [ ] Input field is enabled (not grayed out)
- [ ] You can type in the input field

Once all checkmarks are done:
```
You're ready to chat! 🎉
```

---

**Need more help?** Check the ARCHITECTURE.md or DEPLOYMENT_GUIDE.md files.
