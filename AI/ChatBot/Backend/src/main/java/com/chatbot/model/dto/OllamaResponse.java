package com.chatbot.model.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Response from Ollama API
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class OllamaResponse {
    
    private String model;
    private String response;
    
    @JsonProperty("created_at")
    private String createdAt;
    
    private boolean done;
    
    @JsonProperty("total_duration")
    private long totalDuration;
    
    @JsonProperty("load_duration")
    private long loadDuration;
    
    @JsonProperty("prompt_eval_count")
    private int promptEvalCount;
    
    @JsonProperty("prompt_eval_duration")
    private long promptEvalDuration;
    
    @JsonProperty("eval_count")
    private int evalCount;
    
    @JsonProperty("eval_duration")
    private long evalDuration;
}
