package com.chatbot.service;

import com.chatbot.model.dto.OllamaResponse;
import com.chatbot.model.dto.ModelParameters;
import com.chatbot.common.RequestContext;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;

import java.util.HashMap;
import java.util.Map;

/**
 * Service for communicating with Ollama LLM server
 * 
 * Handles:
 * - Sending prompts to Ollama
 * - Receiving and processing responses
 * - Error handling and timeouts
 * - Request ID propagation for tracing
 */
@Service
@Slf4j
public class OllamaService {
    
    private final RestTemplate restTemplate;
    
    @Value("${ollama.api.url:http://localhost:11434}")
    private String ollamaUrl;
    
    @Value("${ollama.api.timeout:300}")
    private int timeoutSeconds;

    public OllamaService(RestTemplate restTemplate) {
        this.restTemplate = restTemplate;
    }

    /**
     * Send prompt to Ollama and get response
     * 
     * @param model Model name (e.g., "llama2:13b-chat", "llama3:latest", "mistral:latest")
     * @param prompt The user's message/prompt
     * @param parameters Model generation parameters (temperature, top_p, etc.)
     * @return Full response text from Ollama
     */
    public String generateResponse(String model, String prompt, ModelParameters parameters) {
        String requestId = RequestContext.getRequestId();
        long startTime = System.currentTimeMillis();
        
        // Validate model name - ensure it includes version tag
        String validatedModel = validateModelName(model);
        
        // Use default parameters if not provided
        if (parameters == null) {
            parameters = new ModelParameters();
        }
        
        try {
            log.info("Calling Ollama API with model: {} (validated: {}), temp: {}, top_p: {}", 
                model, validatedModel, parameters.getTemperature(), parameters.getTopP());
            
            // Prepare request payload
            Map<String, Object> requestBody = new HashMap<>();
            requestBody.put("model", validatedModel);
            requestBody.put("prompt", prompt);
            requestBody.put("stream", false);
            
            // Add generation parameters
            requestBody.put("temperature", parameters.getTemperature());
            requestBody.put("top_p", parameters.getTopP());
            requestBody.put("top_k", parameters.getTopK());
            requestBody.put("num_predict", parameters.getMaxTokens());
            requestBody.put("repeat_penalty", parameters.getRepetitionPenalty());
            requestBody.put("repeat_last_n", 64); // Context for repetition penalty
            
            // Add seed if provided (for reproducibility)
            if (parameters.getSeed() != null) {
                requestBody.put("seed", parameters.getSeed());
            }
            
            // Create HTTP headers
            HttpHeaders headers = new HttpHeaders();
            headers.setContentType(MediaType.APPLICATION_JSON);
            headers.set("X-Request-ID", requestId);
            
            HttpEntity<Map<String, Object>> request = new HttpEntity<>(requestBody, headers);
            
            // Call Ollama API
            String endpoint = ollamaUrl + "/api/generate";
            OllamaResponse response = restTemplate.postForObject(
                endpoint,
                request,
                OllamaResponse.class
            );
            
            if (response == null) {
                throw new RuntimeException("No response from Ollama API");
            }
            
            long processingTime = System.currentTimeMillis() - startTime;
            log.info("Ollama response received in {}ms, tokens: eval_count={}, prompt_eval_count={}", 
                processingTime, 
                response.getEvalCount(), 
                response.getPromptEvalCount());
            
            return response.getResponse();
            
        } catch (Exception e) {
            long processingTime = System.currentTimeMillis() - startTime;
            log.error("Error calling Ollama API after {}ms: {}", processingTime, e.getMessage(), e);
            throw new RuntimeException("Failed to generate response from Ollama: " + e.getMessage(), e);
        }
    }

    /**
     * Check if Ollama server is available
     */
    public boolean isAvailable() {
        try {
            String endpoint = ollamaUrl + "/api/tags";
            Object response = restTemplate.getForObject(endpoint, Object.class);
            log.debug("Ollama server is available");
            return true;
        } catch (Exception e) {
            log.warn("Ollama server is not available: {}", e.getMessage());
            return false;
        }
    }

    /**
     * Validate and normalize model name
     * Maps short names to full model names with version tags
     * 
     * @param model Model name provided by user
     * @return Validated model name with version tag
     */
    private String validateModelName(String model) {
        if (model == null || model.isEmpty()) {
            return "llama2:13b-chat"; // Default model
        }
        
        // If model already contains version tag (has :), return as-is
        if (model.contains(":")) {
            return model;
        }
        
        // Map short names to full model names
        switch (model.toLowerCase()) {
            case "llama2":
                return "llama2:13b-chat";
            case "llama3":
                return "llama3:latest";
            case "mistral":
                return "mistral:latest";
            default:
                // Return as-is if not recognized
                log.warn("Unknown model: {}, using as-is", model);
                return model;
        }
    }
}
