package com.chatbot.service;

import com.chatbot.model.dto.ChatRequest;
import com.chatbot.model.dto.ChatResponse;
import com.chatbot.model.entity.ChatMessage;
import com.chatbot.model.entity.Conversation;
import com.chatbot.repository.ConversationRepository;
import com.chatbot.common.RequestContext;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;
import java.util.NoSuchElementException;
import java.util.stream.Collectors;

/**
 * Service layer for chat business logic
 * 
 * Responsibilities:
 * - Handle chat requests and responses
 * - Manage conversation lifecycle
 * - Persist messages to database
 * - Coordinate with OllamaService
 * - Include request ID in all messages for tracing
 */
@Service
@Slf4j
@Transactional
public class ChatService {
    
    private final ConversationRepository conversationRepository;
    private final OllamaService ollamaService;

    public ChatService(ConversationRepository conversationRepository, OllamaService ollamaService) {
        this.conversationRepository = conversationRepository;
        this.ollamaService = ollamaService;
    }

    /**
     * Process a chat message and get response from Ollama
     */
    public ChatResponse processMessage(ChatRequest request) {
        String requestId = RequestContext.getRequestId();
        long startTime = System.currentTimeMillis();
        
        log.info("Processing chat message for conversation: {}", request.getConversationId());
        
        // Find or throw
        Conversation conversation = conversationRepository.findById(request.getConversationId())
            .orElseThrow(() -> new NoSuchElementException(
                "Conversation not found: " + request.getConversationId()
            ));
        
        // Save user message
        ChatMessage userMessage = ChatMessage.builder()
            .conversation(conversation)
            .role(ChatMessage.MessageRole.USER)
            .content(request.getMessage())
            .requestId(requestId)
            .build();
        conversation.getMessages().add(userMessage);
        
        log.debug("User message saved: {}", userMessage.getId());
        
        // Get response from Ollama
        String assistantContent = ollamaService.generateResponse(
            request.getModel(), 
            request.getMessage(),
            request.getParameters()
        );
        
        // Save assistant message
        ChatMessage assistantMessage = ChatMessage.builder()
            .conversation(conversation)
            .role(ChatMessage.MessageRole.ASSISTANT)
            .content(assistantContent)
            .requestId(requestId)
            .build();
        conversation.getMessages().add(assistantMessage);
        
        log.debug("Assistant message saved: {}", assistantMessage.getId());
        
        // Save conversation
        conversationRepository.save(conversation);
        
        long processingTime = System.currentTimeMillis() - startTime;
        log.info("Message processing completed in {}ms", processingTime);
        
        // Build response
        return ChatResponse.builder()
            .messageId(assistantMessage.getId())
            .conversationId(conversation.getId())
            .role(assistantMessage.getRole().toString())
            .content(assistantContent)
            .processingTimeMs(processingTime)
            .requestId(requestId)
            .build();
    }

    /**
     * Create a new conversation
     */
    public Conversation createConversation(String model) {
        String title = "Conversation with " + model;
        
        Conversation conversation = Conversation.builder()
            .title(title)
            .model(model)
            .build();
        
        Conversation saved = conversationRepository.save(conversation);
        log.info("New conversation created: {} (model: {})", saved.getId(), model);
        return saved;
    }

    /**
     * Get conversation history
     */
    public List<ChatResponse> getConversationHistory(String conversationId) {
        Conversation conversation = conversationRepository.findById(conversationId)
            .orElseThrow(() -> new NoSuchElementException(
                "Conversation not found: " + conversationId
            ));
        
        return conversation.getMessages().stream()
            .map(msg -> ChatResponse.builder()
                .messageId(msg.getId())
                .conversationId(conversation.getId())
                .role(msg.getRole().toString())
                .content(msg.getContent())
                .requestId(msg.getRequestId())
                .build()
            )
            .collect(Collectors.toList());
    }

    /**
     * Delete conversation and all messages
     */
    public void deleteConversation(String conversationId) {
        if (!conversationRepository.existsById(conversationId)) {
            throw new NoSuchElementException(
                "Conversation not found: " + conversationId
            );
        }
        
        conversationRepository.deleteById(conversationId);
        log.info("Conversation deleted: {}", conversationId);
    }

    /**
     * Get all conversations
     */
    public List<Conversation> getAllConversations() {
        return conversationRepository.findAllByOrderByUpdatedAtDesc();
    }
}
