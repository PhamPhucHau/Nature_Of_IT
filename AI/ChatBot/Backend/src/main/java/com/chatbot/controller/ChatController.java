package com.chatbot.controller;

import com.chatbot.model.dto.ChatRequest;
import com.chatbot.model.dto.ChatResponse;
import com.chatbot.model.dto.ConversationResponse;
import com.chatbot.model.entity.Conversation;
import com.chatbot.service.ChatService;
import com.chatbot.common.RequestContext;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import jakarta.validation.Valid;
import java.util.List;
import java.util.Map;

/**
 * REST Controller for chat endpoints
 * 
 * All endpoints automatically include request ID for tracing via RequestIdFilter
 */
@RestController
@RequestMapping("/api/chat")
@Slf4j
public class ChatController {
    
    private final ChatService chatService;

    public ChatController(ChatService chatService) {
        this.chatService = chatService;
    }

    /**
     * Send a chat message
     * 
     * POST /api/chat/send
     * Request body:
     * {
     *   "conversationId": "uuid",
     *   "message": "Hello!",
     *   "model": "llama2"
     * }
     */
    @PostMapping("/send")
    public ResponseEntity<ChatResponse> sendMessage(@Valid @RequestBody ChatRequest request) {
        String requestId = RequestContext.getRequestId();
        log.info("Sending message in conversation: {}", request.getConversationId());
        
        try {
            ChatResponse response = chatService.processMessage(request);
            return ResponseEntity.ok(response);
        } catch (Exception e) {
            log.error("Error sending message: {}", e.getMessage(), e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }

    /**
     * Create a new conversation
     * 
     * POST /api/chat/conversations
     * Request body:
     * {
     *   "model": "llama2:13b-chat"
     * }
     */
    @PostMapping("/conversations")
    public ResponseEntity<ConversationResponse> createConversation(@RequestBody Map<String, String> payload) {
        String model = payload.getOrDefault("model", "llama2:13b-chat");
        String requestId = RequestContext.getRequestId();
        log.info("Creating new conversation with model: {} [{}]", model, requestId);
        
        Conversation conversation = chatService.createConversation(model);
        ConversationResponse response = ConversationResponse.from(conversation, requestId);
        return ResponseEntity.status(HttpStatus.CREATED).body(response);
    }

    /**
     * Get conversation history
     * 
     * GET /api/chat/history/{conversationId}
     */
    @GetMapping("/history/{conversationId}")
    public ResponseEntity<List<ChatResponse>> getHistory(@PathVariable String conversationId) {
        String requestId = RequestContext.getRequestId();
        log.info("Fetching history for conversation: {}", conversationId);
        
        try {
            List<ChatResponse> history = chatService.getConversationHistory(conversationId);
            return ResponseEntity.ok(history);
        } catch (Exception e) {
            log.error("Error fetching history: {}", e.getMessage());
            return ResponseEntity.status(HttpStatus.NOT_FOUND).build();
        }
    }

    /**
     * Delete conversation
     * 
     * DELETE /api/chat/history/{conversationId}
     */
    @DeleteMapping("/history/{conversationId}")
    public ResponseEntity<Void> deleteConversation(@PathVariable String conversationId) {
        String requestId = RequestContext.getRequestId();
        log.info("Deleting conversation: {}", conversationId);
        
        try {
            chatService.deleteConversation(conversationId);
            return ResponseEntity.noContent().build();
        } catch (Exception e) {
            log.error("Error deleting conversation: {}", e.getMessage());
            return ResponseEntity.status(HttpStatus.NOT_FOUND).build();
        }
    }

    /**
     * Get all conversations
     * 
     * GET /api/chat/conversations
     */
    @GetMapping("/conversations")
    public ResponseEntity<List<Conversation>> getAllConversations() {
        String requestId = RequestContext.getRequestId();
        log.debug("Fetching all conversations");
        
        List<Conversation> conversations = chatService.getAllConversations();
        return ResponseEntity.ok(conversations);
    }

    /**
     * Health check endpoint
     * 
     * GET /api/chat/health
     */
    @GetMapping("/health")
    public ResponseEntity<Map<String, String>> health() {
        String requestId = RequestContext.getRequestId();
        return ResponseEntity.ok(Map.of(
            "status", "UP",
            "requestId", requestId
        ));
    }
}
