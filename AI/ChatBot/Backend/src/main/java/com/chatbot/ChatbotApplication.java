package com.chatbot;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.ComponentScan;

/**
 * Main entry point for the ChatBot application.
 * 
 * Features:
 * - Local LLM integration with Ollama
 * - Chat history persistence with JPA
 * - Global request ID tracing (Anylink middleware)
 * - RESTful API endpoints
 * - CORS security configuration
 */
@SpringBootApplication
@ComponentScan(basePackages = {"com.chatbot"})
public class ChatbotApplication {

    public static void main(String[] args) {
        SpringApplication.run(ChatbotApplication.class, args);
    }
}
