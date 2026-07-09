package com.chatbot.model.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Model generation parameters for controlling LLM output
 * 
 * These parameters control how the model generates text:
 * - Temperature: Controls randomness (0=deterministic, 1=balanced, 2=very random)
 * - Top-p (nucleus sampling): Selects from smallest set of tokens reaching probability p
 * - Top-k: Limits sampling to top k most likely tokens
 * - Max tokens: Maximum length of generated response
 * - Min tokens: Minimum tokens to generate before stopping
 * - Repetition penalty: Penalizes repeated tokens/phrases
 * - Frequency penalty: Reduces repeated words based on frequency
 * - Presence penalty: Encourages new topics instead of repetition
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class ModelParameters {
    
    // Temperature: Controls randomness in output
    // Range: 0.0 - 2.0 (typically 0.0 - 1.0)
    // 0.0 = always pick most likely token (deterministic)
    // 0.7 = balanced (default)
    // 1.0+ = more random and creative
    @Builder.Default
    private Double temperature = 0.7;
    
    // Top-p (nucleus sampling): Only consider tokens with cumulative probability up to p
    // Range: 0.0 - 1.0
    // 0.9 = consider top 90% probability mass
    // 1.0 = consider all tokens
    @Builder.Default
    private Double topP = 0.9;
    
    // Top-k: Only consider top k most likely tokens
    // Range: 1 - 1000
    // 40 = consider only top 40 tokens
    // Helps prevent low-probability nonsense
    @Builder.Default
    private Integer topK = 40;
    
    // Maximum tokens to generate
    // Range: 1 - 4096+
    // 2048 = max length of response
    @Builder.Default
    private Integer maxTokens = 2048;
    
    // Minimum tokens to generate before stopping
    // Range: 0 - 1024
    // Ensures minimum response length
    @Builder.Default
    private Integer minTokens = 0;
    
    // Repetition penalty: Penalizes repeated tokens
    // Range: 1.0 - 2.0
    // 1.0 = no penalty
    // 1.1 = slight penalty for repetition
    // 2.0 = strong penalty (strongly discourage repetition)
    @Builder.Default
    private Double repetitionPenalty = 1.1;
    
    // Frequency penalty: Reduces repeated words based on frequency
    // Range: -2.0 - 2.0
    // 0.0 = no penalty
    // Positive = reduce frequent words
    // Negative = encourage frequent words
    @Builder.Default
    private Double frequencyPenalty = 0.0;
    
    // Presence penalty: Encourages new topics instead of repetition
    // Range: -2.0 - 2.0
    // 0.0 = no penalty
    // Positive = encourage new topics
    // Negative = discourage new topics
    @Builder.Default
    private Double presencePenalty = 0.0;
    
    // Enable sampling (if false, uses greedy decoding)
    @Builder.Default
    private Boolean doSample = true;
    
    // Seed for reproducible outputs (when supported)
    private Long seed;
    
    /**
     * Validate parameters are within acceptable ranges
     */
    public boolean isValid() {
        if (temperature == null || temperature < 0.0 || temperature > 2.0) {
            return false;
        }
        if (topP == null || topP < 0.0 || topP > 1.0) {
            return false;
        }
        if (topK == null || topK < 1 || topK > 1000) {
            return false;
        }
        if (maxTokens == null || maxTokens < 1 || maxTokens > 4096) {
            return false;
        }
        if (minTokens == null || minTokens < 0 || minTokens > 1024) {
            return false;
        }
        if (repetitionPenalty == null || repetitionPenalty < 1.0 || repetitionPenalty > 2.0) {
            return false;
        }
        if (frequencyPenalty == null || frequencyPenalty < -2.0 || frequencyPenalty > 2.0) {
            return false;
        }
        if (presencePenalty == null || presencePenalty < -2.0 || presencePenalty > 2.0) {
            return false;
        }
        return true;
    }
    
    /**
     * Get default parameters for a specific style
     */
    public static ModelParameters getDefaults(String style) {
        switch (style.toLowerCase()) {
            case "creative":
                return ModelParameters.builder()
                    .temperature(1.2)
                    .topP(0.95)
                    .topK(50)
                    .maxTokens(2048)
                    .repetitionPenalty(1.2)
                    .frequencyPenalty(0.1)
                    .presencePenalty(0.1)
                    .build();
            case "balanced":
                return ModelParameters.builder()
                    .temperature(0.7)
                    .topP(0.9)
                    .topK(40)
                    .maxTokens(2048)
                    .repetitionPenalty(1.1)
                    .frequencyPenalty(0.0)
                    .presencePenalty(0.0)
                    .build();
            case "precise":
                return ModelParameters.builder()
                    .temperature(0.3)
                    .topP(0.8)
                    .topK(20)
                    .maxTokens(2048)
                    .repetitionPenalty(1.0)
                    .frequencyPenalty(-0.2)
                    .presencePenalty(-0.2)
                    .build();
            case "deterministic":
                return ModelParameters.builder()
                    .temperature(0.0)
                    .topP(1.0)
                    .topK(1)
                    .maxTokens(2048)
                    .repetitionPenalty(1.0)
                    .frequencyPenalty(0.0)
                    .presencePenalty(0.0)
                    .doSample(false)
                    .build();
            default:
                return ModelParameters.builder().build();
        }
    }
}
