package com.chatbot.model.entity;

import jakarta.persistence.*;
import lombok.*;
import java.time.LocalDateTime;

/**
 * ChatMessage entity - represents a single message in a conversation
 */
@Entity
@Table(name = "chat_messages", indexes = {
    @Index(name = "idx_conversation_id", columnList = "conversation_id"),
    @Index(name = "idx_created_at", columnList = "created_at")
})
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class ChatMessage {
    
    @Id
    @GeneratedValue(strategy = GenerationType.UUID)
    private String id;
    
    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "conversation_id", nullable = false)
    private Conversation conversation;
    
    @Enumerated(EnumType.STRING)
    @Column(nullable = false)
    private MessageRole role;
    
    @Column(columnDefinition = "TEXT", nullable = false)
    private String content;
    
    @Column(name = "request_id")
    private String requestId;
    
    @Column(nullable = false, updatable = false, name = "created_at")
    private LocalDateTime createdAt;
    
    @PrePersist
    protected void onCreate() {
        createdAt = LocalDateTime.now();
    }
    
    public enum MessageRole {
        USER, ASSISTANT, SYSTEM
    }
}
