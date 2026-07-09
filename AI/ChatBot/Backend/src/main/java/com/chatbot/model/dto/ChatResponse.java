package com.chatbot.model.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Response DTO for chat messages
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class ChatResponse {
    
    private String messageId;
    private String conversationId;
    private String role;
    private String content;
    private long processingTimeMs;
    private String requestId;
}
