/**
 * @file timer.h
 * @brief Timer interrupt handler for real-time systems
 * 
 * This module provides timer interrupt handling capabilities for the robot-like
 * human OS. It manages periodic interrupts and elapsed time tracking.
 * 
 * @author Robot OS Team
 * @date 2026
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Timer configuration structure
 * 
 * Defines the configuration for a timer interrupt handler.
 */
typedef struct {
    uint32_t interval_ms;           /**< Timer interval in milliseconds */
    uint32_t tick_count;            /**< Number of ticks elapsed */
    bool is_running;                /**< Timer running state */
    uint32_t total_elapsed_ms;      /**< Total elapsed time in milliseconds */
} TimerConfig;

/**
 * @brief Timer interrupt handler callback type
 * 
 * Function pointer type for timer interrupt callbacks.
 */
typedef void (*TimerCallback)(void);

/**
 * @brief Initialize the timer system
 * 
 * Sets up the timer interrupt handler with the specified interval.
 * 
 * @param interval_ms Timer interval in milliseconds
 * @return true if initialization successful, false otherwise
 * 
 * @example
 * if (timer_init(10)) {
 *     printf("Timer initialized with 10ms interval\n");
 * }
 */
bool timer_init(uint32_t interval_ms);

/**
 * @brief Start the timer
 * 
 * Begins timer interrupt generation.
 * 
 * @return true if timer started successfully, false otherwise
 */
bool timer_start(void);

/**
 * @brief Stop the timer
 * 
 * Stops timer interrupt generation.
 * 
 * @return true if timer stopped successfully, false otherwise
 */
bool timer_stop(void);

/**
 * @brief Get current timer configuration
 * 
 * Retrieves the current timer configuration and state.
 * 
 * @return Pointer to current timer configuration
 */
TimerConfig* timer_get_config(void);

/**
 * @brief Get elapsed time in milliseconds
 * 
 * Returns the total elapsed time since timer initialization.
 * 
 * @return Elapsed time in milliseconds
 */
uint32_t timer_get_elapsed_ms(void);

/**
 * @brief Get current tick count
 * 
 * Returns the number of timer ticks that have occurred.
 * 
 * @return Current tick count
 */
uint32_t timer_get_tick_count(void);

/**
 * @brief Reset timer
 * 
 * Resets the timer counters to zero.
 * 
 * @return true if reset successful, false otherwise
 */
bool timer_reset(void);

/**
 * @brief Register timer callback
 * 
 * Registers a callback function to be called on each timer interrupt.
 * 
 * @param callback Function pointer to callback
 * @return true if callback registered successfully, false otherwise
 */
bool timer_register_callback(TimerCallback callback);

/**
 * @brief Unregister timer callback
 * 
 * Removes the registered timer callback.
 * 
 * @return true if callback unregistered successfully, false otherwise
 */
bool timer_unregister_callback(void);

/**
 * @brief Simulate timer interrupt (for testing)
 * 
 * Manually triggers a timer interrupt for testing purposes.
 * This is useful for unit testing without actual hardware.
 * 
 * @return true if interrupt simulated successfully, false otherwise
 */
bool timer_simulate_interrupt(void);

/**
 * @brief Get timer status string
 * 
 * Returns a human-readable status string for the timer.
 * 
 * @return Status string (e.g., "Running", "Stopped")
 */
const char* timer_get_status(void);

#endif /* TIMER_H */
