import React from 'react'
import '../styles/ModelSelector.css'

/**
 * ModelSelector component
 * 
 * Allows users to choose between different Ollama models
 * Models are based on user's local Ollama installation:
 * - llama2:13b-chat (13B parameter Llama 2 model)
 * - llama3:latest (Latest Llama 3 model)
 * - mistral:latest (Latest Mistral model)
 */
const ModelSelector = ({ selectedModel, onModelChange, disabled }) => {
  const models = [
    { id: 'llama2:13b-chat', label: 'Llama 2 (13B)', description: 'Fast and accurate' },
    { id: 'llama3:latest', label: 'Llama 3', description: 'Latest and powerful' },
    { id: 'mistral:latest', label: 'Mistral', description: 'High-quality responses' }
  ]

  return (
    <div className="model-selector">
      <label htmlFor="model-select">Model:</label>
      <select
        id="model-select"
        value={selectedModel}
        onChange={(e) => onModelChange(e.target.value)}
        disabled={disabled}
        className="model-select"
      >
        {models.map(model => (
          <option key={model.id} value={model.id}>
            {model.label}
          </option>
        ))}
      </select>
      <span className="model-description">
        {models.find(m => m.id === selectedModel)?.description}
      </span>
    </div>
  )
}

export default ModelSelector
