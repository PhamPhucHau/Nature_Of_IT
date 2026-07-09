package com.chatbot.common;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * Thread-local storage for request context.
 * 
 * Holds the global request ID that flows through entire application stack.
 * Each thread gets its own RequestContext instance via ThreadLocal.
 * 
 * Usage:
 *   RequestContext.setRequestId("req_a1b2c3d4e5f6");
 *   String id = RequestContext.getRequestId(); // "req_a1b2c3d4e5f6"
 *   RequestContext.clear(); // Clean up thread-local
 */
public class RequestContext {
    private static final ThreadLocal<String> requestIdHolder = new ThreadLocal<>();

    /**
     * Set the request ID for current thread
     */
    public static void setRequestId(String requestId) {
        requestIdHolder.set(requestId);
    }

    /**
     * Get the request ID for current thread
     */
    public static String getRequestId() {
        return requestIdHolder.get();
    }

    /**
     * Clear thread-local storage (important for thread pool reuse)
     */
    public static void clear() {
        requestIdHolder.remove();
    }

    /**
     * Check if request ID is set
     */
    public static boolean hasRequestId() {
        return requestIdHolder.get() != null;
    }
}
