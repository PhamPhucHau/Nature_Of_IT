package com.chatbot.repository;

import com.chatbot.model.entity.Conversation;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

/**
 * Repository for Conversation entity - handles database operations
 */
@Repository
public interface ConversationRepository extends JpaRepository<Conversation, String> {
    
    /**
     * Find all conversations ordered by most recent
     */
    List<Conversation> findAllByOrderByUpdatedAtDesc();
}
