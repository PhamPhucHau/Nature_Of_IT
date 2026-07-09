package com.chatbot.model.dto;

import com.chatbot.model.entity.Conversation;
import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.time.LocalDateTime;
import java.util.Collections;
import java.util.List;

/**
 * Response DTO for conversation operations
 * Wraps Conversation entity and adds metadata like requestId
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class ConversationResponse {
    
    private String id;
    private String title;
    private String model;
    private List<MessageDTO> messages;
    private LocalDateTime createdAt;
    private LocalDateTime updatedAt;
    private String requestId;
    
    /**
     * Create ConversationResponse from Conversation entity
     */
    public static ConversationResponse from(Conversation conversation, String requestId) {
        return ConversationResponse.builder()
            .id(conversation.getId())
            .title(conversation.getTitle())
            .model(conversation.getModel())
            .messages(Collections.emptyList())
            .createdAt(conversation.getCreatedAt())
            .updatedAt(conversation.getUpdatedAt())
            .requestId(requestId)
            .build();
    }
    
    /**
     * Simple message DTO for response
     */
    @Data
    @NoArgsConstructor
    @AllArgsConstructor
    public static class MessageDTO {
        private String id;
        private String role;
        private String content;
    }
}
