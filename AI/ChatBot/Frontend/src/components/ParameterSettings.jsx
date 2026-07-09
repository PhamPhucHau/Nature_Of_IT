import React, { useState } from 'react'
import '../styles/ParameterSettings.css'

/**
 * ParameterSettings component
 * 
 * Allows users to adjust LLM generation parameters:
 * - Temperature: Randomness control
 * - Top-p: Nucleus sampling
 * - Top-k: Token limit
 * - Max tokens: Response length limit
 * - Repetition penalty: Discourage repetition
 */
const ParameterSettings = ({ parameters, onParametersChange, disabled }) => {
  const [isExpanded, setIsExpanded] = useState(false)
  const [localParams, setLocalParams] = useState(parameters || {
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

  const presets = {
    creative: {
      temperature: 1.2,
      topP: 0.95,
      topK: 50,
      maxTokens: 2048,
      repetitionPenalty: 1.2,
      frequencyPenalty: 0.1,
      presencePenalty: 0.1,
      description: 'Creative and diverse responses'
    },
    balanced: {
      temperature: 0.7,
      topP: 0.9,
      topK: 40,
      maxTokens: 2048,
      repetitionPenalty: 1.1,
      frequencyPenalty: 0.0,
      presencePenalty: 0.0,
      description: 'Balanced quality and coherence'
    },
    precise: {
      temperature: 0.3,
      topP: 0.8,
      topK: 20,
      maxTokens: 2048,
      repetitionPenalty: 1.0,
      frequencyPenalty: -0.2,
      presencePenalty: -0.2,
      description: 'Focused and factual responses'
    },
    deterministic: {
      temperature: 0.0,
      topP: 1.0,
      topK: 1,
      maxTokens: 2048,
      repetitionPenalty: 1.0,
      frequencyPenalty: 0.0,
      presencePenalty: 0.0,
      doSample: false,
      description: 'Consistent and reproducible'
    }
  }

  const handleParameterChange = (param, value) => {
    const updated = { ...localParams, [param]: value }
    setLocalParams(updated)
    onParametersChange(updated)
  }

  const applyPreset = (presetName) => {
    const preset = presets[presetName]
    const { description, ...params } = preset
    setLocalParams(params)
    onParametersChange(params)
  }

  const resetToDefaults = () => {
    const defaults = presets.balanced
    const { description, ...params } = defaults
    setLocalParams(params)
    onParametersChange(params)
  }

  return (
    <div className="parameter-settings">
      <button 
        className="settings-toggle"
        onClick={() => setIsExpanded(!isExpanded)}
        disabled={disabled}
        title="Adjust model generation parameters"
      >
        ⚙️ {isExpanded ? 'Hide' : 'Show'} Parameters
      </button>

      {isExpanded && (
        <div className="settings-panel">
          <div className="settings-header">
            <h3>Generation Parameters</h3>
            <span className="settings-info">Fine-tune model output behavior</span>
          </div>

          {/* Presets */}
          <div className="presets-section">
            <label>Quick Presets:</label>
            <div className="preset-buttons">
              {Object.entries(presets).map(([key, preset]) => (
                <button
                  key={key}
                  className={`preset-btn ${localParams.temperature === preset.temperature ? 'active' : ''}`}
                  onClick={() => applyPreset(key)}
                  disabled={disabled}
                  title={preset.description}
                >
                  {key.charAt(0).toUpperCase() + key.slice(1)}
                </button>
              ))}
            </div>
          </div>

          {/* Main Parameters */}
          <div className="parameters-grid">
            {/* Temperature */}
            <div className="parameter-group">
              <label htmlFor="temperature">
                Temperature
                <span className="range-hint">Randomness</span>
              </label>
              <input
                id="temperature"
                type="range"
                min="0"
                max="2"
                step="0.1"
                value={localParams.temperature}
                onChange={(e) => handleParameterChange('temperature', parseFloat(e.target.value))}
                disabled={disabled}
                className="slider"
              />
              <div className="parameter-display">
                <span className="value">{localParams.temperature.toFixed(2)}</span>
                <span className="description">
                  {localParams.temperature < 0.3 && 'Very deterministic'}
                  {localParams.temperature >= 0.3 && localParams.temperature < 0.7 && 'Focused'}
                  {localParams.temperature >= 0.7 && localParams.temperature < 1.0 && 'Balanced'}
                  {localParams.temperature >= 1.0 && 'Creative'}
                </span>
              </div>
            </div>

            {/* Top-P */}
            <div className="parameter-group">
              <label htmlFor="topP">
                Top-P (Nucleus Sampling)
                <span className="range-hint">Token diversity</span>
              </label>
              <input
                id="topP"
                type="range"
                min="0"
                max="1"
                step="0.05"
                value={localParams.topP}
                onChange={(e) => handleParameterChange('topP', parseFloat(e.target.value))}
                disabled={disabled}
                className="slider"
              />
              <div className="parameter-display">
                <span className="value">{localParams.topP.toFixed(2)}</span>
                <span className="description">Top {(localParams.topP * 100).toFixed(0)}% probability</span>
              </div>
            </div>

            {/* Top-K */}
            <div className="parameter-group">
              <label htmlFor="topK">
                Top-K
                <span className="range-hint">Token limit</span>
              </label>
              <input
                id="topK"
                type="range"
                min="1"
                max="100"
                step="1"
                value={localParams.topK}
                onChange={(e) => handleParameterChange('topK', parseInt(e.target.value))}
                disabled={disabled}
                className="slider"
              />
              <div className="parameter-display">
                <span className="value">{localParams.topK}</span>
                <span className="description">Top {localParams.topK} tokens</span>
              </div>
            </div>

            {/* Max Tokens */}
            <div className="parameter-group">
              <label htmlFor="maxTokens">
                Max Tokens
                <span className="range-hint">Response length</span>
              </label>
              <input
                id="maxTokens"
                type="range"
                min="100"
                max="4096"
                step="100"
                value={localParams.maxTokens}
                onChange={(e) => handleParameterChange('maxTokens', parseInt(e.target.value))}
                disabled={disabled}
                className="slider"
              />
              <div className="parameter-display">
                <span className="value">{localParams.maxTokens}</span>
                <span className="description">Max {localParams.maxTokens} tokens</span>
              </div>
            </div>

            {/* Repetition Penalty */}
            <div className="parameter-group">
              <label htmlFor="repetitionPenalty">
                Repetition Penalty
                <span className="range-hint">Avoid repetition</span>
              </label>
              <input
                id="repetitionPenalty"
                type="range"
                min="1"
                max="2"
                step="0.1"
                value={localParams.repetitionPenalty}
                onChange={(e) => handleParameterChange('repetitionPenalty', parseFloat(e.target.value))}
                disabled={disabled}
                className="slider"
              />
              <div className="parameter-display">
                <span className="value">{localParams.repetitionPenalty.toFixed(2)}</span>
                <span className="description">
                  {localParams.repetitionPenalty <= 1.0 && 'Off'}
                  {localParams.repetitionPenalty > 1.0 && localParams.repetitionPenalty <= 1.3 && 'Light'}
                  {localParams.repetitionPenalty > 1.3 && 'Strong'}
                </span>
              </div>
            </div>

            {/* Frequency Penalty */}
            <div className="parameter-group">
              <label htmlFor="frequencyPenalty">
                Frequency Penalty
                <span className="range-hint">Common words</span>
              </label>
              <input
                id="frequencyPenalty"
                type="range"
                min="-2"
                max="2"
                step="0.1"
                value={localParams.frequencyPenalty}
                onChange={(e) => handleParameterChange('frequencyPenalty', parseFloat(e.target.value))}
                disabled={disabled}
                className="slider"
              />
              <div className="parameter-display">
                <span className="value">{localParams.frequencyPenalty.toFixed(2)}</span>
                <span className="description">
                  {localParams.frequencyPenalty < 0 && 'Encourage repeat'}
                  {localParams.frequencyPenalty === 0 && 'Neutral'}
                  {localParams.frequencyPenalty > 0 && 'Reduce repeat'}
                </span>
              </div>
            </div>
          </div>

          {/* Advanced Options */}
          <div className="advanced-section">
            <label className="checkbox-label">
              <input
                type="checkbox"
                checked={localParams.doSample}
                onChange={(e) => handleParameterChange('doSample', e.target.checked)}
                disabled={disabled}
              />
              <span>Use Sampling (if unchecked: use greedy decoding)</span>
            </label>
          </div>

          {/* Reset Button */}
          <div className="settings-actions">
            <button
              className="btn-reset"
              onClick={resetToDefaults}
              disabled={disabled}
            >
              ↺ Reset to Defaults
            </button>
          </div>

          {/* Info Panel */}
          <div className="info-panel">
            <h4>Parameter Guide:</h4>
            <ul>
              <li><strong>Temperature:</strong> Higher = more random and creative, Lower = more focused</li>
              <li><strong>Top-P:</strong> Nucleus sampling - select from smallest set of tokens reaching probability p</li>
              <li><strong>Top-K:</strong> Limits sampling to top k most likely tokens</li>
              <li><strong>Max Tokens:</strong> Maximum length of the model's response</li>
              <li><strong>Repetition Penalty:</strong> Discourages the model from repeating tokens</li>
              <li><strong>Frequency Penalty:</strong> Reduces repeated words based on frequency</li>
            </ul>
          </div>
        </div>
      )}
    </div>
  )
}

export default ParameterSettings
