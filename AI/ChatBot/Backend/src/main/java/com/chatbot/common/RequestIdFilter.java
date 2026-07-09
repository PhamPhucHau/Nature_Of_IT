package com.chatbot.common;

import jakarta.servlet.Filter;
import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.ServletRequest;
import jakarta.servlet.ServletResponse;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;
import org.springframework.web.util.ContentCachingResponseWrapper;

import java.io.IOException;
import java.util.UUID;

/**
 * Anylink Middleware - Global Request ID Generator and Propagator
 * 
 * This filter (servlet filter) is the entry point for all HTTP requests.
 * It generates a unique request ID and stores it in ThreadLocal storage.
 * The request ID flows through the entire application stack and is included
 * in all log messages via MDC (Mapped Diagnostic Context).
 * 
 * Request ID Format: req_xxxxxxxx (req_ prefix + 8-char UUID)
 * Example: req_a1b2c3d4
 * 
 * Features:
 * - Generates globally unique request IDs
 * - Stores in thread-local for easy access
 * - Sets MDC for logging integration
 * - Propagates via X-Request-ID header
 * - Cleans up after request completes
 */
@Component
@Slf4j
public class RequestIdFilter implements Filter {

    private static final String REQUEST_ID_HEADER = "X-Request-ID";
    private static final String REQUEST_ID_PREFIX = "req_";
    private static final String MDC_KEY = "requestId";

    @Override
    public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain)
            throws IOException, ServletException {
        
        HttpServletRequest httpRequest = (HttpServletRequest) request;
        HttpServletResponse httpResponse = (HttpServletResponse) response;

        // Generate or extract request ID
        String requestId = extractOrGenerateRequestId(httpRequest);

        try {
            // Store in thread-local
            RequestContext.setRequestId(requestId);
            
            // Set MDC for logging (SLF4J will include this in logs)
            org.slf4j.MDC.put(MDC_KEY, requestId);

            // Add to response header
            httpResponse.setHeader(REQUEST_ID_HEADER, requestId);

            log.debug("Request started: {} {} [{}]", 
                httpRequest.getMethod(), 
                httpRequest.getRequestURI(),
                requestId);

            // Continue filter chain
            chain.doFilter(request, response);

            log.debug("Request completed: {} {} [{}]", 
                httpRequest.getMethod(), 
                httpRequest.getRequestURI(),
                requestId);

        } finally {
            // Clean up thread-local (important for thread pool reuse)
            RequestContext.clear();
            org.slf4j.MDC.remove(MDC_KEY);
        }
    }

    /**
     * Extract request ID from header or generate new one
     */
    private String extractOrGenerateRequestId(HttpServletRequest request) {
        String headerValue = request.getHeader(REQUEST_ID_HEADER);
        
        if (headerValue != null && !headerValue.isEmpty()) {
            return headerValue;
        }

        // Generate new request ID: req_xxxxxxxx
        String uuid = UUID.randomUUID().toString().substring(0, 8);
        return REQUEST_ID_PREFIX + uuid;
    }
}
