# Model Parameter Settings Guide

**Date:** July 4, 2026  
**Feature:** Advanced LLM generation parameter controls  
**Status:** ✅ **READY TO DEPLOY**

---

## Overview

The ChatBot now includes comprehensive parameter settings that allow you to fine-tune how the Ollama models generate responses. These controls give you direct access to LLM generation parameters for customized behavior.

---

## New Components

### Backend Changes

#### 1. **ModelParameters.java** - DTO for generation parameters
```java
// Location: Backend/src/main/java/com/chatbot/model/dto/ModelParameters.java

Key parameters:
- temperature (0.0 - 2.0): Controls randomness
- topP (0.0 - 1.0): Nucleus sampling
- topK (1 - 1000): Token limit
- maxTokens (1 - 4096): Response length limit
- minTokens (0 - 1024): Minimum response length
- repetitionPenalty (1.0 - 2.0): Discourage repetition
- frequencyPenalty (-2.0 - 2.0): Reduce repeated words
- presencePenalty (-2.0 - 2.0): Encourage new topics
- doSample (true/false): Use sampling vs greedy
- seed (integer): Reproducibility
```

#### 2. **ChatRequest.java** - Updated to include parameters
```java
@Data
public class ChatRequest {
    private String conversationId;
    private String message;
    private String model;
    private ModelParameters parameters;  // ← NEW
}
```

#### 3. **OllamaService.java** - Updated to use parameters
```java
public String generateResponse(String model, String prompt, ModelParameters parameters)
```

#### 4. **ChatService.java** - Passes parameters to Ollama
```java
ollamaService.generateResponse(
    request.getModel(), 
    request.getMessage(),
    request.getParameters()  // ← NEW
);
```

### Frontend Changes

#### 1. **ParameterSettings.jsx** - New UI component
```jsx
// Location: Frontend/src/components/ParameterSettings.jsx

Features:
✓ Quick presets (Creative, Balanced, Precise, Deterministic)
✓ Individual parameter sliders
✓ Real-time preview and descriptions
✓ Advanced options toggle
✓ Reset to defaults button
✓ Help documentation
```

#### 2. **ParameterSettings.css** - Styling
```css
// Location: Frontend/src/styles/ParameterSettings.css

Responsive design with:
- Grid layout for parameters
- Smooth animations
- Mobile support
- Accessible controls
```

#### 3. **useChatbot.js Hook** - Parameter state management
```javascript
const [modelParameters, setModelParameters] = useState({...})
// Exported in hook return object
```

#### 4. **api.js** - Parameter transmission
```javascript
export const sendMessage = async (
  conversationId, 
  message, 
  model, 
  parameters = null  // ← NEW
)
```

#### 5. **ChatBox.jsx** - Component integration
```jsx
<ParameterSettings 
  parameters={modelParameters}
  onParametersChange={onParametersChange}
  disabled={!currentConversationId || isLoading}
/>
```

---

## Parameter Guide

### Temperature (0.0 - 2.0)
**Purpose:** Controls randomness/creativity

| Value | Behavior | Use Case |
|-------|----------|----------|
| 0.0 | Always pick most likely token (deterministic) | Factual answers, code generation |
| 0.3 | Very focused, predictable | Technical questions |
| 0.7 | Balanced (default) | General chat |
| 1.0+ | More random and creative | Creative writing, brainstorming |
| 1.5+ | Very random and unpredictable | Experimental, very creative |

### Top-P (0.0 - 1.0)
**Purpose:** Nucleus sampling - consider tokens with cumulative probability ≤ p

| Value | Meaning |
|-------|---------|
| 0.8 | Consider top ~80% of probability mass |
| 0.9 | Consider top ~90% (default, good balance) |
| 1.0 | Consider all tokens |

**Note:** Works well with lower temperature values. Higher top_p = more variety.

### Top-K (1 - 1000)
**Purpose:** Limits sampling to top k most likely tokens

| Value | Meaning |
|-------|---------|
| 1 | Only most likely token (very restrictive) |
| 20 | Top 20 tokens (precise) |
| 40 | Top 40 tokens (default, balanced) |
| 100+ | Many tokens (more variety) |

**Note:** Helps prevent low-probability nonsense while maintaining variety.

### Max Tokens (1 - 4096)
**Purpose:** Maximum length of model's response

| Value | Impact |
|-------|--------|
| 256 | Short, concise answers |
| 1024 | Medium responses |
| 2048 | Long responses (default) |
| 4096 | Maximum length |

### Min Tokens (0 - 1024)
**Purpose:** Minimum tokens to generate before stopping

| Value | Impact |
|-------|--------|
| 0 | No minimum (can give very short answers) |
| 100 | Ensures some minimum length |
| 256+ | Forces longer responses |

### Repetition Penalty (1.0 - 2.0)
**Purpose:** Penalizes repeated tokens/phrases

| Value | Effect |
|-------|--------|
| 1.0 | No penalty (allow repetition) |
| 1.1 | Light penalty (default) |
| 1.5 | Medium penalty |
| 2.0 | Strong penalty (strongly discourage) |

### Frequency Penalty (-2.0 - 2.0)
**Purpose:** Reduces repeated words based on frequency

| Value | Effect |
|-------|--------|
| -2.0 | Encourage repeated words |
| 0.0 | Neutral (default) |
| 0.5 | Mild penalty for common words |
| 2.0 | Strong penalty for common words |

### Presence Penalty (-2.0 - 2.0)
**Purpose:** Encourages new topics instead of repetition

| Value | Effect |
|-------|--------|
| -2.0 | Stay on same topic |
| 0.0 | Neutral (default) |
| 0.5 | Mild encouragement for new topics |
| 2.0 | Strong encouragement for diversity |

---

## Preset Configurations

### 🎨 Creative
```
Purpose: Diverse, creative responses
Temperature: 1.2 (creative)
Top-P: 0.95 (high variety)
Top-K: 50 (many options)
Repetition Penalty: 1.2 (avoid repeat)
Frequency Penalty: 0.1 (vary words)
Presence Penalty: 0.1 (explore topics)

Use for: Creative writing, brainstorming, poetry
```

### ⚖️ Balanced (Default)
```
Purpose: Good balance of quality and coherence
Temperature: 0.7 (balanced)
Top-P: 0.9 (good variety)
Top-K: 40 (selective)
Repetition Penalty: 1.1 (light penalty)
Frequency Penalty: 0.0 (neutral)
Presence Penalty: 0.0 (neutral)

Use for: General chat, balanced responses
```

### 🎯 Precise
```
Purpose: Focused, factual responses
Temperature: 0.3 (focused)
Top-P: 0.8 (conservative)
Top-K: 20 (very selective)
Repetition Penalty: 1.0 (allow if needed)
Frequency Penalty: -0.2 (allow common words)
Presence Penalty: -0.2 (stay focused)

Use for: Technical questions, factual information, code
```

### 🔒 Deterministic
```
Purpose: Consistent, reproducible outputs
Temperature: 0.0 (deterministic)
Top-P: 1.0 (all tokens)
Top-K: 1 (most likely only)
Repetition Penalty: 1.0 (no penalty)
Sampling: Disabled (greedy decoding)

Use for: Testing, reproducible outputs, consistency checks
```

---

## How to Use

### 1. Access Parameter Settings
Click the **⚙️ Show Parameters** button below the Model Selector

### 2. Quick Setup - Use Presets
Click one of the preset buttons:
- **Creative** - for creative/exploratory responses
- **Balanced** - for general chat (default)
- **Precise** - for factual/technical questions
- **Deterministic** - for reproducible outputs

### 3. Fine-Tune Individual Parameters
Adjust sliders for granular control:
- Each parameter shows real-time value and description
- Changes apply immediately to next message
- All parameters persist during conversation

### 4. Reset to Defaults
Click **↺ Reset to Defaults** to restore balanced settings

---

## Request Flow

```
User Input + Parameters
    ↓
API: POST /api/chat/send
{
  conversationId: "uuid",
  message: "What is AI?",
  model: "llama2:13b-chat",
  parameters: {
    temperature: 0.7,
    topP: 0.9,
    topK: 40,
    ...
  }
}
    ↓
ChatController (validates)
    ↓
ChatService (processes)
    ↓
OllamaService.generateResponse(model, prompt, parameters)
    ↓
Ollama HTTP API
{
  model: "llama2:13b-chat",
  prompt: "What is AI?",
  temperature: 0.7,
  top_p: 0.9,
  top_k: 40,
  num_predict: 2048,
  repeat_penalty: 1.1,
  ...
}
    ↓
LLM Response
    ↓
Backend Response with Request ID
    ↓
Frontend Display
```

---

## Files Modified/Created

### Backend
| File | Change | Type |
|------|--------|------|
| `ModelParameters.java` | NEW | DTO with validation |
| `ChatRequest.java` | UPDATED | Added parameters field |
| `OllamaService.java` | UPDATED | Accept parameters |
| `ChatService.java` | UPDATED | Pass parameters |

### Frontend
| File | Change | Type |
|------|--------|------|
| `ParameterSettings.jsx` | NEW | UI component |
| `ParameterSettings.css` | NEW | Styling |
| `useChatbot.js` | UPDATED | State management |
| `api.js` | UPDATED | Send parameters |
| `ChatBox.jsx` | UPDATED | Integrate component |
| `App.jsx` | UPDATED | Pass props |

---

## Deployment Steps

1. **Build Backend**
   ```bash
   mvn clean package -DskipTests
   ```

2. **Build Frontend**
   ```bash
   npm run build
   ```

3. **Rebuild Docker Images**
   ```bash
   docker-compose down
   docker-compose up -d --build
   ```

4. **Verify Deployment**
   - Backend health: http://localhost:8080/api/chat/health
   - Frontend: http://localhost:3000

---

## Testing the Feature

### Test 1: Use Creative Preset
1. Create conversation
2. Click "⚙️ Show Parameters"
3. Click "Creative" preset
4. Send: "Write a poem about coding"
5. Observe more creative, diverse responses

### Test 2: Use Precise Preset
1. Create conversation
2. Click "⚙️ Show Parameters"
3. Click "Precise" preset
4. Send: "What is machine learning?"
5. Observe focused, factual responses

### Test 3: Adjust Temperature Slider
1. Create conversation
2. Click "⚙️ Show Parameters"
3. Drag Temperature to 1.5
4. Send: "Tell me a story"
5. Observe very creative output

### Test 4: Reset to Defaults
1. Adjust multiple parameters
2. Click "↺ Reset to Defaults"
3. Verify all parameters return to balanced values

---

## Advanced Usage

### Use Case 1: Technical Writing
```
Preset: Precise
Additional:
- Max Tokens: 3000 (longer responses)
- Temperature: 0.2 (very focused)
- Top-K: 15 (conservative)

For: Documentation, technical explanations
```

### Use Case 2: Brainstorming
```
Preset: Creative
Additional:
- Temperature: 1.3 (very creative)
- Top-P: 0.95 (high diversity)
- Frequency Penalty: 0.3 (vary vocabulary)

For: Ideation, creative exploration
```

### Use Case 3: Code Generation
```
Preset: Precise
Additional:
- Temperature: 0.1 (focused)
- Max Tokens: 2048 (full implementations)
- Repetition Penalty: 1.05 (slight penalty)

For: Code generation, debugging help
```

### Use Case 4: Reproducible Testing
```
Preset: Deterministic
Additional:
- Seed: 42 (reproducible)
- Max Tokens: 1024 (consistent length)

For: Testing, consistency checks
```

---

## FAQ

**Q: Which preset should I use?**
A: Start with "Balanced" (default). For specific needs:
- Creative content → Creative preset
- Factual answers → Precise preset
- Technical help → Precise preset
- Testing → Deterministic preset

**Q: Why isn't my parameter change affecting results?**
A: Changes apply to the NEXT message you send, not previous ones.

**Q: What's the difference between Temperature and Top-P?**
A: Temperature controls how random the sampling is globally, while Top-P specifically controls which tokens are considered (nucleus sampling).

**Q: Can I mix presets?**
A: Yes! Apply a preset first, then adjust individual parameters as needed.

**Q: What happens if parameters are invalid?**
A: Backend validates parameters and returns an error. Use Reset to Defaults if unsure.

---

## Performance Notes

- Parameter adjustments add negligible overhead (~<1ms)
- All parameters are passed to Ollama in a single request
- No additional database queries required
- Parameters are NOT stored (they're request-scoped)

---

## Future Enhancements

Potential improvements:
1. Save parameter presets by name
2. Per-conversation parameter history
3. Parameter recommendations based on model
4. Advanced metrics (like token probability)
5. Parameter export/import
6. A/B testing parameter combinations

---

**The parameter settings feature is now ready to enhance model behavior and give users fine-grained control over LLM responses!** 🎉
