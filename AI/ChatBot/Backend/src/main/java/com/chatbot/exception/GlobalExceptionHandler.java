package com.chatbot.exception;

import com.chatbot.common.RequestContext;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import java.util.HashMap;
import java.util.Map;
import java.util.NoSuchElementException;

/**
 * Global exception handler for all REST endpoints
 * 
 * Provides consistent error responses with request ID for tracing
 */
@RestControllerAdvice
@Slf4j
public class GlobalExceptionHandler {

    /**
     * Handle validation errors
     */
    @ExceptionHandler(MethodArgumentNotValidException.class)
    public ResponseEntity<Map<String, Object>> handleValidationException(
            MethodArgumentNotValidException ex) {
        
        Map<String, Object> response = buildErrorResponse(
            HttpStatus.BAD_REQUEST,
            "Validation failed",
            ex.getBindingResult().getFieldError().getDefaultMessage()
        );
        
        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(response);
    }

    /**
     * Handle not found errors
     */
    @ExceptionHandler(NoSuchElementException.class)
    public ResponseEntity<Map<String, Object>> handleNotFound(NoSuchElementException ex) {
        Map<String, Object> response = buildErrorResponse(
            HttpStatus.NOT_FOUND,
            "Resource not found",
            ex.getMessage()
        );
        
        log.warn("Resource not found: {}", ex.getMessage());
        return ResponseEntity.status(HttpStatus.NOT_FOUND).body(response);
    }

    /**
     * Handle generic runtime exceptions
     */
    @ExceptionHandler(RuntimeException.class)
    public ResponseEntity<Map<String, Object>> handleRuntimeException(RuntimeException ex) {
        Map<String, Object> response = buildErrorResponse(
            HttpStatus.INTERNAL_SERVER_ERROR,
            "Internal server error",
            ex.getMessage()
        );
        
        log.error("Runtime exception: ", ex);
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(response);
    }

    /**
     * Handle all other exceptions
     */
    @ExceptionHandler(Exception.class)
    public ResponseEntity<Map<String, Object>> handleException(Exception ex) {
        Map<String, Object> response = buildErrorResponse(
            HttpStatus.INTERNAL_SERVER_ERROR,
            "An unexpected error occurred",
            ex.getMessage()
        );
        
        log.error("Unexpected exception: ", ex);
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(response);
    }

    /**
     * Build standardized error response with request ID
     */
    private Map<String, Object> buildErrorResponse(
            HttpStatus status, 
            String message, 
            String details) {
        
        Map<String, Object> response = new HashMap<>();
        response.put("status", status.value());
        response.put("message", message);
        response.put("details", details);
        response.put("requestId", RequestContext.getRequestId());
        response.put("timestamp", System.currentTimeMillis());
        
        return response;
    }
}
