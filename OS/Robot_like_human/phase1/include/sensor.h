/**
 * @file sensor.h
 * @brief Sensor data structures and interfaces
 * 
 * This module defines the data structures and interfaces for sensor handling
 * in the robot-like human OS. It provides abstractions for various sensor types.
 * 
 * @author Robot OS Team
 * @date 2026
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Maximum number of sensors
 */
#define MAX_SENSORS 16

/**
 * @brief Sensor types enumeration
 */
typedef enum {
    SENSOR_TYPE_TEMPERATURE,    /**< Temperature sensor */
    SENSOR_TYPE_HUMIDITY,       /**< Humidity sensor */
    SENSOR_TYPE_LIGHT,          /**< Light/illumination sensor */
    SENSOR_TYPE_SOUND,          /**< Sound/audio sensor */
    SENSOR_TYPE_MOTION,         /**< Motion/accelerometer sensor */
    SENSOR_TYPE_DISTANCE,       /**< Distance/ultrasonic sensor */
    SENSOR_TYPE_PRESSURE,       /**< Pressure sensor */
    SENSOR_TYPE_GYROSCOPE,      /**< Gyroscope sensor */
    SENSOR_TYPE_MAGNETOMETER,   /**< Magnetometer sensor */
    SENSOR_TYPE_UNKNOWN         /**< Unknown sensor type */
} SensorType;

/**
 * @brief Sensor status enumeration
 */
typedef enum {
    SENSOR_STATUS_OK,           /**< Sensor operating normally */
    SENSOR_STATUS_WARNING,      /**< Sensor warning state */
    SENSOR_STATUS_ERROR,        /**< Sensor error state */
    SENSOR_STATUS_OFFLINE       /**< Sensor offline */
} SensorStatus;

/**
 * @brief Single sensor reading
 * 
 * Represents a single reading from a sensor with timestamp and status.
 */
typedef struct {
    uint32_t sensor_id;         /**< Unique sensor identifier */
    SensorType type;            /**< Type of sensor */
    float value;                /**< Sensor reading value */
    float min_value;            /**< Minimum expected value */
    float max_value;            /**< Maximum expected value */
    SensorStatus status;        /**< Current sensor status */
    uint32_t timestamp_ms;      /**< Timestamp in milliseconds */
    uint32_t read_count;        /**< Number of times read */
} SensorReading;

/**
 * @brief Sensor configuration
 * 
 * Configuration parameters for a sensor.
 */
typedef struct {
    uint32_t sensor_id;         /**< Unique sensor identifier */
    SensorType type;            /**< Type of sensor */
    float min_value;            /**< Minimum expected value */
    float max_value;            /**< Maximum expected value */
    uint32_t sample_rate_hz;    /**< Sampling rate in Hz */
    bool enabled;               /**< Whether sensor is enabled */
} SensorConfig;

/**
 * @brief Multi-sensor reading collection
 * 
 * Represents readings from multiple sensors at a point in time.
 */
typedef struct {
    SensorReading readings[MAX_SENSORS];  /**< Array of sensor readings */
    uint32_t num_sensors;                 /**< Number of active sensors */
    uint32_t timestamp_ms;                /**< Collection timestamp */
    uint32_t collection_id;               /**< Unique collection identifier */
} SensorCollection;

/**
 * @brief Initialize sensor system
 * 
 * Sets up the sensor system and prepares for sensor operations.
 * 
 * @return true if initialization successful, false otherwise
 */
bool sensor_system_init(void);

/**
 * @brief Register a sensor
 * 
 * Registers a new sensor with the system.
 * 
 * @param config Sensor configuration
 * @return Sensor ID if successful, -1 if failed
 * 
 * @example
 * SensorConfig config = {
 *     .type = SENSOR_TYPE_TEMPERATURE,
 *     .min_value = -40.0,
 *     .max_value = 125.0,
 *     .sample_rate_hz = 10
 * };
 * int sensor_id = sensor_register(&config);
 */
int32_t sensor_register(SensorConfig *config);

/**
 * @brief Unregister a sensor
 * 
 * Removes a sensor from the system.
 * 
 * @param sensor_id ID of sensor to unregister
 * @return true if successful, false otherwise
 */
bool sensor_unregister(uint32_t sensor_id);

/**
 * @brief Read sensor value
 * 
 * Reads the current value from a specific sensor.
 * 
 * @param sensor_id ID of sensor to read
 * @param reading Pointer to store the reading
 * @return true if read successful, false otherwise
 */
bool sensor_read(uint32_t sensor_id, SensorReading *reading);

/**
 * @brief Read all sensors
 * 
 * Reads values from all registered sensors.
 * 
 * @param collection Pointer to store the collection
 * @return Number of sensors read, -1 if failed
 */
int32_t sensor_read_all(SensorCollection *collection);

/**
 * @brief Get sensor configuration
 * 
 * Retrieves the configuration of a specific sensor.
 * 
 * @param sensor_id ID of sensor
 * @param config Pointer to store configuration
 * @return true if successful, false otherwise
 */
bool sensor_get_config(uint32_t sensor_id, SensorConfig *config);

/**
 * @brief Update sensor value (for simulation)
 * 
 * Updates the value of a sensor for testing/simulation purposes.
 * 
 * @param sensor_id ID of sensor
 * @param value New sensor value
 * @return true if successful, false otherwise
 */
bool sensor_update_value(uint32_t sensor_id, float value);

/**
 * @brief Get sensor status
 * 
 * Retrieves the current status of a sensor.
 * 
 * @param sensor_id ID of sensor
 * @return Current sensor status
 */
SensorStatus sensor_get_status(uint32_t sensor_id);

/**
 * @brief Get number of registered sensors
 * 
 * Returns the count of currently registered sensors.
 * 
 * @return Number of registered sensors
 */
uint32_t sensor_get_count(void);

/**
 * @brief Get sensor type name
 * 
 * Returns a human-readable name for a sensor type.
 * 
 * @param type Sensor type
 * @return Type name string
 */
const char* sensor_get_type_name(SensorType type);

/**
 * @brief Get sensor status name
 * 
 * Returns a human-readable name for a sensor status.
 * 
 * @param status Sensor status
 * @return Status name string
 */
const char* sensor_get_status_name(SensorStatus status);

/**
 * @brief Shutdown sensor system
 * 
 * Cleans up and shuts down the sensor system.
 * 
 * @return true if successful, false otherwise
 */
bool sensor_system_shutdown(void);

#endif /* SENSOR_H */
