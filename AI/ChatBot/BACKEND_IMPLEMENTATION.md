# Backend Implementation Guide
## Java Spring Boot ChatBot Backend

---

## 📁 Project Setup

### pom.xml Dependencies

```xml
<?xml version="1.0" encoding="UTF-8"?>
<project xmlns="http://maven.apache.org/POM/4.0.0"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 
         http://maven.apache.org/xsd/maven-4.0.0.xsd">
    <modelVersion>4.0.0</modelVersion>

    <groupId>com.chatbot</groupId>
    <artifactId>chatbot-backend</artifactId>
    <version>1.0.0</version>
    <name>ChatBot Backend</name>

    <parent>
        <groupId>org.springframework.boot</groupId>
        <artifactId>spring-boot-starter-parent</artifactId>
        <version>3.1.5</version>
    </parent>

    <properties>
        <java.version>17</java.version>
        <maven.compiler.source>17</maven.compiler.source>
        <maven.compiler.target>17</maven.compiler.target>
    </properties>

    <dependencies>
        <!-- Spring Boot Web -->
        <dependency>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter-web</artifactId>
        </dependency>

        <!-- Spring Boot Data JPA -->
        <dependency>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter-data-jpa</artifactId>
        </dependency>

        <!-- H2 Database (in-memory for development) -->
        <dependency>
            <groupId>com.h2database</groupId>
            <artifactId>h2</artifactId>
            <scope>runtime</scope>
        </dependency>

        <!-- PostgreSQL (for production) -->
        <dependency>
            <groupId>org.postgresql</groupId>
            <artifactId>postgresql</artifactId>
            <version>42.6.0</version>
        </dependency>

        <!-- Lombok (reduce boilerplate) -->
        <dependency>
            <groupId>org.projectlombok</groupId>
            <artifactId>lombok</artifactId>
            <optional>true</optional>
        </dependency>

        <!-- HttpClient for Ollama API calls -->
        <dependency>
            <groupId>org.apache.httpcomponents.client5</groupId>
            <artifactId>httpclient5</artifactId>
            <version>5.2.1</version>
        </dependency>

        <!-- Jackson for JSON -->
        <dependency>
            <groupId>com.fasterxml.jackson.core</groupId>
            <artifactId>jackson-databind</artifactId>
        </dependency>

        <!-- AspectJ for AOP logging -->
        <dependency>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter-aop</artifactId>
        </dependency>

        <!-- Testing -->
        <dependency>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter-test</artifactId>
            <scope>test</scope>
        </dependency>
    </dependencies>

    <build>
        <plugins>
            <plugin>
                <groupId>org.springframework.boot</groupId>
                <artifactId>spring-boot-maven-plugin</artifactId>
            </plugin>
        </plugins>
    </build>
</project>
```

---

## 🛡️ Security & Middleware

### 1. RequestIdFilter (Global Request ID)

```java
package com.chatbot.security;

import jakarta.servlet.*;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.slf4j.MDC;
import org.springframework.stereotype.Component;

import java.io.IOException;
import java.util.UUID;

@Component
public class RequestIdFilter implements Filter {
    
    private static final String REQUEST_ID_HEADER = "X-Request-ID";
    private static final String MDC_REQUEST_ID = "requestId";

    @Override
    public void doFilter(ServletRequest request, ServletResponse response, 
                        FilterChain chain) throws IOException, ServletException {
        HttpServletRequest httpRequest = (HttpServletRequest) request;
        HttpServletResponse httpResponse = (HttpServletResponse) response;
        
        // Get or generate request ID
        String requestId = httpRequest.getHeader(REQUEST_ID_HEADER);
        if (requestId == null || requestId.isEmpty()) {
            requestId = "req_" + UUID.randomUUID().toString().substring(0, 12);
        }
        
        // Store in MDC for logging
        MDC.put(MDC_REQUEST_ID, requestId);
        
        try {
            // Add to response header
            httpResponse.setHeader(REQUEST_ID_HEADER, requestId);
            
            // Continue filter chain
            chain.doFilter(request, response);
        } finally {
            // Clean up MDC
            MDC.remove(MDC_REQUEST_ID);
        }
    }
}
```

### 2. RequestContext (Thread-Local Storage)

```java
package com.chatbot.common;

import org.slf4j.MDC;

public class RequestContext {
    
    private static final ThreadLocal<String> requestIdHolder = new ThreadLocal<>();
    
    public static void setRequestId(String requestId) {
        requestIdHolder.set(requestId);
        MDC.put("requestId", requestId);
    }
    
    public static String getRequestId() {
        return requestIdHolder.get();
    }
    
    public static void clear() {
        requestIdHolder.remove();
        MDC.remove("requestId");
    }
}
```

### 3. SecurityConfig

```java
package com.chatbot.security;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.web.SecurityFilterChain;

@Configuration
@EnableWebSecurity
public class SecurityConfig {

    @Bean
    public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
        http
            .authorizeHttpRequests(authz -> authz
                .requestMatchers("/api/**").permitAll()
                .requestMatchers("/h2-console/**").permitAll()
                .anyRequest().authenticated()
            )
            .csrf().disable()
            .headers().frameOptions().disable();

        return http.build();
    }
}
```

### 4. CorsMvcConfig

```java
package com.chatbot.security;

import org.springframework.context.annotation.Configuration;
import org.springframework.web.servlet.config.annotation.CorsRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;

@Configuration
public class CorsMvcConfig implements WebMvcConfigurer {

    @Override
    public void addCorsMappings(CorsRegistry registry) {
        registry.addMapping("/api/**")
            .allowedOrigins("http://localhost:3000", "http://localhost:5173")
            .allowedMethods("GET", "POST", "PUT", "DELETE", "OPTIONS")
            .allowedHeaders("*")
            .exposedHeaders("X-Request-ID")
            .allowCredentials(true)
            .maxAge(3600);
    }
}
```

---

## 📊 Models & Entities

### 1. ChatMessage Entity

```java
package com.chatbot.model;

import jakarta.persistence.*;
import lombok.*;
import java.time.LocalDateTime;

@Entity
@Table(name = "chat_messages")
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class ChatMessage {
    
    @Id
    @GeneratedValue(strategy = GenerationType.UUID)
    private String id;
    
    @Column(name = "conversation_id", nullable = false)
    private String conversationId;
    
    @Column(name = "user_message", columnDefinition = "TEXT")
    private String userMessage;
    
    @Column(name = "bot_response", columnDefinition = "TEXT")
    private String botResponse;
    
    @Column(name = "model_used", nullable = false)
    private String modelUsed;
    
    @Column(name = "request_id")
    private String requestId;
    
    @Column(name = "response_time_ms")
    private Long responseTimeMs;
    
    @Column(name = "created_at", nullable = false)
    private LocalDateTime createdAt;
    
    @Column(name = "tokens_used")
    private Integer tokensUsed;
    
    @PrePersist
    protected void onCreate() {
        createdAt = LocalDateTime.now();
    }
}
```

### 2. ChatRequest DTO

```java
package com.chatbot.model;

import lombok.*;

@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class ChatRequest {
    private String message;
    private String model;
    private String conversationId;
    private Integer maxTokens;
    private Double temperature;
}
```

### 3. ChatResponse DTO

```java
package com.chatbot.model;

import lombok.*;

@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class ChatResponse {
    private String response;
    private String model;
    private String conversationId;
    private Long responseTimeMs;
    private Integer tokensUsed;
    private String requestId;
}
```

---

## 🔄 Service Layer

### 1. ChatService Interface

```java
package com.chatbot.service;

import com.chatbot.model.ChatRequest;
import com.chatbot.model.ChatResponse;
import java.util.List;

public interface ChatService {
    ChatResponse sendMessage(ChatRequest request);
    List<ChatResponse> getConversationHistory(String conversationId);
    void clearConversation(String conversationId);
}
```

### 2. ChatServiceImpl

```java
package com.chatbot.service.impl;

import com.chatbot.model.*;
import com.chatbot.repository.ChatHistoryRepository;
import com.chatbot.service.ChatService;
import com.chatbot.service.OllamaService;
import com.chatbot.common.RequestContext;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;

@Service
@Slf4j
public class ChatServiceImpl implements ChatService {
    
    private final ChatHistoryRepository chatHistoryRepository;
    private final OllamaService ollamaService;
    
    public ChatServiceImpl(ChatHistoryRepository chatHistoryRepository,
                         OllamaService ollamaService) {
        this.chatHistoryRepository = chatHistoryRepository;
        this.ollamaService = ollamaService;
    }

    @Override
    public ChatResponse sendMessage(ChatRequest request) {
        String requestId = RequestContext.getRequestId();
        log.info("[{}] ChatServiceImpl.sendMessage() - Processing message", requestId);
        
        long startTime = System.currentTimeMillis();
        
        try {
            // Generate conversation ID if not provided
            String conversationId = request.getConversationId() != null 
                ? request.getConversationId() 
                : "conv_" + UUID.randomUUID().toString().substring(0, 8);
            
            log.debug("[{}] Conversation ID: {}", requestId, conversationId);
            
            // Call Ollama API
            log.info("[{}] Calling OllamaService for model: {}", 
                    requestId, request.getModel());
            
            OllamaResponse ollamaResponse = ollamaService.generate(
                request.getMessage(),
                request.getModel(),
                request.getTemperature(),
                request.getMaxTokens()
            );
            
            log.debug("[{}] Ollama response received", requestId);
            
            // Save to database
            ChatMessage chatMessage = ChatMessage.builder()
                .conversationId(conversationId)
                .userMessage(request.getMessage())
                .botResponse(ollamaResponse.getResponse())
                .modelUsed(request.getModel())
                .requestId(requestId)
                .responseTimeMs(ollamaResponse.getResponseTimeMs())
                .tokensUsed(ollamaResponse.getTokensUsed())
                .build();
            
            chatHistoryRepository.save(chatMessage);
            log.info("[{}] Message saved to database", requestId);
            
            long totalTime = System.currentTimeMillis() - startTime;
            
            return ChatResponse.builder()
                .response(ollamaResponse.getResponse())
                .model(request.getModel())
                .conversationId(conversationId)
                .responseTimeMs(totalTime)
                .tokensUsed(ollamaResponse.getTokensUsed())
                .requestId(requestId)
                .build();
                
        } catch (Exception e) {
            log.error("[{}] Error in sendMessage: {}", requestId, e.getMessage(), e);
            throw new RuntimeException("Failed to process chat message", e);
        }
    }

    @Override
    public List<ChatResponse> getConversationHistory(String conversationId) {
        String requestId = RequestContext.getRequestId();
        log.info("[{}] Getting conversation history for: {}", requestId, conversationId);
        
        return chatHistoryRepository.findByConversationIdOrderByCreatedAtDesc(conversationId)
            .stream()
            .map(msg -> ChatResponse.builder()
                .response(msg.getBotResponse())
                .model(msg.getModelUsed())
                .conversationId(msg.getConversationId())
                .responseTimeMs(msg.getResponseTimeMs())
                .requestId(msg.getRequestId())
                .build())
            .collect(Collectors.toList());
    }

    @Override
    public void clearConversation(String conversationId) {
        String requestId = RequestContext.getRequestId();
        log.warn("[{}] Clearing conversation: {}", requestId, conversationId);
        
        chatHistoryRepository.deleteByConversationId(conversationId);
    }
}
```

### 3. OllamaService

```java
package com.chatbot.service;

import com.fasterxml.jackson.databind.ObjectMapper;
import lombok.extern.slf4j.Slf4j;
import org.apache.hc.client5.http.classic.HttpClient;
import org.apache.hc.client5.http.classic.methods.HttpPost;
import org.apache.hc.client5.http.impl.classic.HttpClients;
import org.apache.hc.core5.http.io.entity.StringEntity;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import com.chatbot.common.RequestContext;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

@Service
@Slf4j
public class OllamaService {
    
    @Value("${ollama.api.url:http://localhost:11434}")
    private String ollamaUrl;
    
    private final ObjectMapper objectMapper;
    private final HttpClient httpClient;
    
    public OllamaService() {
        this.objectMapper = new ObjectMapper();
        this.httpClient = HttpClients.createDefault();
    }

    public OllamaResponse generate(String prompt, String model, 
                                   Double temperature, Integer maxTokens) {
        String requestId = RequestContext.getRequestId();
        long startTime = System.currentTimeMillis();
        
        try {
            log.info("[{}] OllamaService.generate() - Model: {}", requestId, model);
            
            String url = ollamaUrl + "/api/generate";
            HttpPost post = new HttpPost(url);
            post.setHeader("Content-Type", "application/json");
            
            // Build request
            Map<String, Object> requestBody = new HashMap<>();
            requestBody.put("model", model);
            requestBody.put("prompt", prompt);
            requestBody.put("stream", false);
            if (temperature != null) {
                requestBody.put("temperature", temperature);
            }
            if (maxTokens != null) {
                requestBody.put("num_predict", maxTokens);
            }
            
            String jsonRequest = objectMapper.writeValueAsString(requestBody);
            post.setEntity(new StringEntity(jsonRequest));
            
            log.debug("[{}] Sending request to Ollama: {}", requestId, url);
            
            // Execute request
            var response = httpClient.execute(post, httpResponse -> {
                String content = new String(httpResponse.getEntity().getContent().readAllBytes());
                Map<String, Object> responseMap = objectMapper.readValue(
                    content, Map.class
                );
                return responseMap;
            });
            
            String generatedText = (String) response.get("response");
            long responseTime = System.currentTimeMillis() - startTime;
            
            log.info("[{}] Ollama response received in {}ms", requestId, responseTime);
            log.debug("[{}] Generated text: {}", requestId, 
                     generatedText.substring(0, Math.min(100, generatedText.length())));
            
            return OllamaResponse.builder()
                .response(generatedText)
                .responseTimeMs(responseTime)
                .tokensUsed((Integer) response.getOrDefault("eval_count", 0))
                .build();
                
        } catch (IOException e) {
            log.error("[{}] Error calling Ollama API: {}", requestId, e.getMessage(), e);
            throw new RuntimeException("Failed to call Ollama API", e);
        }
    }
}
```

---

## 🎯 Controller Layer

### ChatController

```java
package com.chatbot.controller;

import com.chatbot.model.ChatRequest;
import com.chatbot.model.ChatResponse;
import com.chatbot.service.ChatService;
import com.chatbot.common.RequestContext;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import java.util.List;

@RestController
@RequestMapping("/api/chat")
@CrossOrigin(origins = {"http://localhost:3000", "http://localhost:5173"})
@Slf4j
public class ChatController {
    
    private final ChatService chatService;
    
    public ChatController(ChatService chatService) {
        this.chatService = chatService;
    }

    @PostMapping("/send")
    public ResponseEntity<ChatResponse> sendMessage(@RequestBody ChatRequest request) {
        String requestId = RequestContext.getRequestId();
        log.info("[{}] ChatController.sendMessage() - Received request", requestId);
        
        ChatResponse response = chatService.sendMessage(request);
        
        log.info("[{}] Sending response to client", requestId);
        return ResponseEntity.ok(response);
    }

    @GetMapping("/history/{conversationId}")
    public ResponseEntity<List<ChatResponse>> getHistory(
            @PathVariable String conversationId) {
        String requestId = RequestContext.getRequestId();
        log.info("[{}] ChatController.getHistory() for conversation: {}", 
                requestId, conversationId);
        
        List<ChatResponse> history = chatService.getConversationHistory(conversationId);
        
        return ResponseEntity.ok(history);
    }

    @DeleteMapping("/history/{conversationId}")
    public ResponseEntity<Void> clearHistory(
            @PathVariable String conversationId) {
        String requestId = RequestContext.getRequestId();
        log.warn("[{}] ChatController.clearHistory() for conversation: {}", 
                requestId, conversationId);
        
        chatService.clearConversation(conversationId);
        
        return ResponseEntity.ok().build();
    }
}
```

---

## 📦 Repository Layer

### ChatHistoryRepository

```java
package com.chatbot.repository;

import com.chatbot.model.ChatMessage;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;
import java.util.List;

@Repository
public interface ChatHistoryRepository extends JpaRepository<ChatMessage, String> {
    List<ChatMessage> findByConversationIdOrderByCreatedAtDesc(String conversationId);
    void deleteByConversationId(String conversationId);
}
```

---

## ⚙️ Configuration

### application.yml

```yaml
spring:
  application:
    name: chatbot-backend
  
  datasource:
    url: jdbc:h2:mem:chatbot
    driverClassName: org.h2.Driver
    username: sa
    password: 
  
  jpa:
    database-platform: org.hibernate.dialect.H2Dialect
    hibernate:
      ddl-auto: create-drop
    show-sql: false
    properties:
      hibernate.format_sql: true

  h2:
    console:
      enabled: true
      path: /h2-console

server:
  port: 8080
  servlet:
    context-path: /

ollama:
  api:
    url: http://localhost:11434

logging:
  level:
    root: INFO
    com.chatbot: DEBUG
  pattern:
    console: "[%d{yyyy-MM-dd HH:mm:ss}] [%X{requestId}] %p %c{1} - %m%n"
    file: "[%d{yyyy-MM-dd HH:mm:ss}] [%X{requestId}] %p %c{1} - %m%n"
  file:
    name: logs/chatbot.log
    max-size: 10MB
    max-history: 10
```

---

## 🚀 Running the Backend

```bash
# Build
mvn clean package

# Run
java -jar target/chatbot-backend-1.0.0.jar

# Or with Maven
mvn spring-boot:run

# Check health
curl http://localhost:8080/actuator/health
```

Backend will be available at `http://localhost:8080`
API endpoints ready at `http://localhost:8080/api/chat`
