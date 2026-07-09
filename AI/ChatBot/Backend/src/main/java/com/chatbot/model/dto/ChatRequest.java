package com.chatbot.model.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import jakarta.validation.constraints.NotBlank;

/**
 * Request DTO for sending a chat message
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class ChatRequest {
    
    @NotBlank(message = "Conversation ID is required")
    private String conversationId;
    
    @NotBlank(message = "Message content is required")
    private String message;
    
    @NotBlank(message = "Model name is required")
    private String model;
    
    // Model generation parameters (optional, uses defaults if not provided)
    @Builder.Default
    private ModelParameters parameters = new ModelParameters();
}
