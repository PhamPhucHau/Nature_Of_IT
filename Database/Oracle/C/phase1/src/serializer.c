/*
 * serializer.c - Serialization Functions Implementation
 * 
 * This file implements functions for serializing and deserializing data
 * to/from binary format. The binary format is designed to be:
 * - Compact: Minimal storage overhead
 * - Portable: Works across different architectures
 * - Verifiable: Includes headers for format validation
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "serializer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * ============================================================================
 * DEBUG MACROS
 * ============================================================================
 */

#define DEBUG 1

#if DEBUG
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

/*
 * ============================================================================
 * ENDIANNESS HANDLING
 * ============================================================================
 */

/*
 * Explanation of Endianness:
 * 
 * Big-endian (network byte order):
 *   Most significant byte first
 *   Example: 0x12345678 → [0x12, 0x34, 0x56, 0x78]
 * 
 * Little-endian:
 *   Least significant byte first
 *   Example: 0x12345678 → [0x78, 0x56, 0x34, 0x12]
 * 
 * We use little-endian for consistency across platforms.
 */

/*
 * uint32_to_bytes - Convert uint32_t to 4 bytes (little-endian)
 * 
 * Parameters:
 *   value: The value to convert
 *   bytes: Output buffer (must be at least 4 bytes)
 */
static void uint32_to_bytes(uint32_t value, uint8_t *bytes) {
    bytes[0] = (uint8_t)(value & 0xFF);           // LSB
    bytes[1] = (uint8_t)((value >> 8) & 0xFF);
    bytes[2] = (uint8_t)((value >> 16) & 0xFF);
    bytes[3] = (uint8_t)((value >> 24) & 0xFF);   // MSB
}

/*
 * bytes_to_uint32 - Convert 4 bytes to uint32_t (little-endian)
 * 
 * Parameters:
 *   bytes: Input buffer (must be at least 4 bytes)
 * 
 * Returns:
 *   The converted value
 */
static uint32_t bytes_to_uint32(const uint8_t *bytes) {
    return ((uint32_t)bytes[0]) |
           (((uint32_t)bytes[1]) << 8) |
           (((uint32_t)bytes[2]) << 16) |
           (((uint32_t)bytes[3]) << 24);
}

/*
 * ============================================================================
 * BASIC SERIALIZATION FUNCTIONS
 * ============================================================================
 */

int serializer_write_uint32(FILE *file, uint32_t value) {
    if (!file) {
        ERROR_PRINT("File pointer is NULL\n");
        return -1;
    }
    
    DEBUG_PRINT("Writing uint32: %u\n", value);
    
    /* Convert to bytes */
    uint8_t bytes[4];
    uint32_to_bytes(value, bytes);
    
    /* Write to file */
    size_t written = fwrite(bytes, 1, 4, file);
    if (written != 4) {
        ERROR_PRINT("Failed to write uint32: wrote %zu bytes\n", written);
        return -1;
    }
    
    return 0;
}

int serializer_read_uint32(FILE *file, uint32_t *value) {
    if (!file || !value) {
        ERROR_PRINT("Invalid arguments: file=%p, value=%p\n", file, value);
        return -1;
    }
    
    DEBUG_PRINT("Reading uint32\n");
    
    /* Read bytes from file */
    uint8_t bytes[4];
    size_t read = fread(bytes, 1, 4, file);
    if (read != 4) {
        ERROR_PRINT("Failed to read uint32: read %zu bytes\n", read);
        return -1;
    }
    
    /* Convert from bytes */
    *value = bytes_to_uint32(bytes);
    
    DEBUG_PRINT("Read uint32: %u\n", *value);
    return 0;
}

int serializer_write_string(FILE *file, const char *str) {
    if (!file || !str) {
        ERROR_PRINT("Invalid arguments: file=%p, str=%p\n", file, str);
        return -1;
    }
    
    DEBUG_PRINT("Writing string: %s\n", str);
    
    /* Get string length */
    uint32_t length = (uint32_t)strlen(str);
    
    /* Write length */
    if (serializer_write_uint32(file, length) != 0) {
        ERROR_PRINT("Failed to write string length\n");
        return -1;
    }
    
    /* Write string data */
    size_t written = fwrite(str, 1, length, file);
    if (written != length) {
        ERROR_PRINT("Failed to write string data: wrote %zu bytes\n", written);
        return -1;
    }
    
    DEBUG_PRINT("String written successfully\n");
    return 0;
}

int serializer_read_string(FILE *file, char **str) {
    if (!file || !str) {
        ERROR_PRINT("Invalid arguments: file=%p, str=%p\n", file, str);
        return -1;
    }
    
    DEBUG_PRINT("Reading string\n");
    
    /* Read length */
    uint32_t length;
    if (serializer_read_uint32(file, &length) != 0) {
        ERROR_PRINT("Failed to read string length\n");
        return -1;
    }
    
    DEBUG_PRINT("String length: %u\n", length);
    
    /* Allocate memory for string (length + 1 for null terminator) */
    *str = (char *)malloc(length + 1);
    if (!*str) {
        ERROR_PRINT("Failed to allocate memory for string\n");
        return -1;
    }
    
    /* Read string data */
    size_t read = fread(*str, 1, length, file);
    if (read != length) {
        ERROR_PRINT("Failed to read string data: read %zu bytes\n", read);
        free(*str);
        *str = NULL;
        return -1;
    }
    
    /* Add null terminator */
    (*str)[length] = '\0';
    
    DEBUG_PRINT("String read: %s\n", *str);
    return 0;
}

int serializer_write_bytes(FILE *file, const void *data, size_t size) {
    if (!file || !data) {
        ERROR_PRINT("Invalid arguments: file=%p, data=%p\n", file, data);
        return -1;
    }
    
    DEBUG_PRINT("Writing %zu bytes\n", size);
    
    size_t written = fwrite(data, 1, size, file);
    if (written != size) {
        ERROR_PRINT("Failed to write bytes: wrote %zu bytes\n", written);
        return -1;
    }
    
    return 0;
}

int serializer_read_bytes(FILE *file, void *data, size_t size) {
    if (!file || !data) {
        ERROR_PRINT("Invalid arguments: file=%p, data=%p\n", file, data);
        return -1;
    }
    
    DEBUG_PRINT("Reading %zu bytes\n", size);
    
    size_t read = fread(data, 1, size, file);
    if (read != size) {
        ERROR_PRINT("Failed to read bytes: read %zu bytes\n", read);
        return -1;
    }
    
    return 0;
}

/*
 * ============================================================================
 * FILE HEADER FUNCTIONS
 * ============================================================================
 */

int serializer_write_header(FILE *file) {
    if (!file) {
        ERROR_PRINT("File pointer is NULL\n");
        return -1;
    }
    
    DEBUG_PRINT("Writing file header\n");
    
    /* Write magic number */
    if (serializer_write_uint32(file, SERIALIZER_MAGIC) != 0) {
        ERROR_PRINT("Failed to write magic number\n");
        return -1;
    }
    
    /* Write version */
    if (serializer_write_uint32(file, SERIALIZER_VERSION) != 0) {
        ERROR_PRINT("Failed to write version\n");
        return -1;
    }
    
    DEBUG_PRINT("File header written successfully\n");
    return 0;
}

int serializer_read_header(FILE *file) {
    if (!file) {
        ERROR_PRINT("File pointer is NULL\n");
        return -1;
    }
    
    DEBUG_PRINT("Reading file header\n");
    
    /* Read magic number */
    uint32_t magic;
    if (serializer_read_uint32(file, &magic) != 0) {
        ERROR_PRINT("Failed to read magic number\n");
        return -1;
    }
    
    if (magic != SERIALIZER_MAGIC) {
        ERROR_PRINT("Invalid magic number: 0x%X (expected 0x%X)\n", 
                   magic, SERIALIZER_MAGIC);
        return -1;
    }
    
    /* Read version */
    uint32_t version;
    if (serializer_read_uint32(file, &version) != 0) {
        ERROR_PRINT("Failed to read version\n");
        return -1;
    }
    
    if (version != SERIALIZER_VERSION) {
        ERROR_PRINT("Incompatible version: %u (expected %u)\n", 
                   version, SERIALIZER_VERSION);
        return -1;
    }
    
    DEBUG_PRINT("File header verified successfully\n");
    return 0;
}

/*
 * ============================================================================
 * FILE UTILITY FUNCTIONS
 * ============================================================================
 */

long serializer_get_file_size(const char *filename) {
    if (!filename) {
        ERROR_PRINT("Filename is NULL\n");
        return -1;
    }
    
    DEBUG_PRINT("Getting file size: %s\n", filename);
    
    FILE *file = fopen(filename, "rb");
    if (!file) {
        DEBUG_PRINT("File does not exist: %s\n", filename);
        return -1;
    }
    
    /* Seek to end of file */
    if (fseek(file, 0, SEEK_END) != 0) {
        ERROR_PRINT("Failed to seek to end of file\n");
        fclose(file);
        return -1;
    }
    
    /* Get current position (which is the file size) */
    long size = ftell(file);
    if (size < 0) {
        ERROR_PRINT("Failed to get file position\n");
        fclose(file);
        return -1;
    }
    
    fclose(file);
    
    DEBUG_PRINT("File size: %ld bytes\n", size);
    return size;
}

int serializer_file_exists(const char *filename) {
    if (!filename) {
        ERROR_PRINT("Filename is NULL\n");
        return 0;
    }
    
    DEBUG_PRINT("Checking if file exists: %s\n", filename);
    
    FILE *file = fopen(filename, "rb");
    if (!file) {
        DEBUG_PRINT("File does not exist\n");
        return 0;
    }
    
    fclose(file);
    DEBUG_PRINT("File exists\n");
    return 1;
}
