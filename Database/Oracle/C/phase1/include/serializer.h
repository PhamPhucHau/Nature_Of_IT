/*
 * serializer.h - Serialization Functions Header
 * 
 * This header defines functions for serializing and deserializing data
 * to/from binary format. Serialization is the process of converting
 * in-memory data structures to a byte stream for storage or transmission.
 * 
 * Key Concepts:
 * - Endianness: Byte order (big-endian vs little-endian)
 * - Binary format: Compact representation of data
 * - Type safety: Ensuring correct data types during serialization
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdio.h>    // For FILE
#include <stdint.h>   // For uint32_t, uint8_t
#include <stddef.h>   // For size_t

/*
 * ============================================================================
 * CONSTANTS
 * ============================================================================
 */

/* Magic number to identify our file format */
#define SERIALIZER_MAGIC 0x4B5642  // "KVB" in hex

/* Current serialization format version */
#define SERIALIZER_VERSION 1

/*
 * ============================================================================
 * SERIALIZATION FUNCTIONS
 * ============================================================================
 */

/*
 * serializer_write_uint32 - Write a 32-bit unsigned integer to file
 * 
 * Writes a 32-bit unsigned integer in little-endian format.
 * This ensures consistent byte order across different architectures.
 * 
 * Binary format:
 *   [Byte 0: LSB] [Byte 1] [Byte 2] [Byte 3: MSB]
 * 
 * Parameters:
 *   file: File pointer (must be opened in binary write mode)
 *   value: The value to write
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   FILE *f = fopen("data.bin", "wb");
 *   serializer_write_uint32(f, 42);
 *   fclose(f);
 */
int serializer_write_uint32(FILE *file, uint32_t value);

/*
 * serializer_read_uint32 - Read a 32-bit unsigned integer from file
 * 
 * Reads a 32-bit unsigned integer in little-endian format.
 * 
 * Parameters:
 *   file: File pointer (must be opened in binary read mode)
 *   value: Output parameter for the read value
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   FILE *f = fopen("data.bin", "rb");
 *   uint32_t value;
 *   serializer_read_uint32(f, &value);
 *   printf("Read: %u\n", value);
 *   fclose(f);
 */
int serializer_read_uint32(FILE *file, uint32_t *value);

/*
 * serializer_write_string - Write a string to file
 * 
 * Writes a string in a format that can be read back:
 * - First writes the string length (4 bytes)
 * - Then writes the string data
 * 
 * Binary format:
 *   [4 bytes: length] [length bytes: string data]
 * 
 * Parameters:
 *   file: File pointer (must be opened in binary write mode)
 *   str: The string to write (null-terminated)
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   FILE *f = fopen("data.bin", "wb");
 *   serializer_write_string(f, "Hello, World!");
 *   fclose(f);
 */
int serializer_write_string(FILE *file, const char *str);

/*
 * serializer_read_string - Read a string from file
 * 
 * Reads a string that was written with serializer_write_string.
 * Allocates memory for the string, which must be freed by the caller.
 * 
 * Parameters:
 *   file: File pointer (must be opened in binary read mode)
 *   str: Output parameter for the read string (pointer to char*)
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   FILE *f = fopen("data.bin", "rb");
 *   char *str;
 *   serializer_read_string(f, &str);
 *   printf("Read: %s\n", str);
 *   free(str);
 *   fclose(f);
 */
int serializer_read_string(FILE *file, char **str);

/*
 * serializer_write_bytes - Write raw bytes to file
 * 
 * Writes raw bytes to file. Useful for writing binary data.
 * 
 * Parameters:
 *   file: File pointer (must be opened in binary write mode)
 *   data: Pointer to data to write
 *   size: Number of bytes to write
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   FILE *f = fopen("data.bin", "wb");
 *   uint8_t data[] = {0x01, 0x02, 0x03};
 *   serializer_write_bytes(f, data, 3);
 *   fclose(f);
 */
int serializer_write_bytes(FILE *file, const void *data, size_t size);

/*
 * serializer_read_bytes - Read raw bytes from file
 * 
 * Reads raw bytes from file. The caller must provide a buffer
 * large enough to hold the data.
 * 
 * Parameters:
 *   file: File pointer (must be opened in binary read mode)
 *   data: Buffer to read into
 *   size: Number of bytes to read
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   FILE *f = fopen("data.bin", "rb");
 *   uint8_t data[3];
 *   serializer_read_bytes(f, data, 3);
 *   fclose(f);
 */
int serializer_read_bytes(FILE *file, void *data, size_t size);

/*
 * serializer_write_header - Write file header
 * 
 * Writes a header to identify the file format and version.
 * This helps detect corrupted or incompatible files.
 * 
 * Header format:
 *   [4 bytes: magic number] [4 bytes: version]
 * 
 * Parameters:
 *   file: File pointer (must be opened in binary write mode)
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   FILE *f = fopen("data.bin", "wb");
 *   serializer_write_header(f);
 *   // ... write data ...
 *   fclose(f);
 */
int serializer_write_header(FILE *file);

/*
 * serializer_read_header - Read and verify file header
 * 
 * Reads the file header and verifies it matches our format.
 * Returns an error if the file is not in the correct format.
 * 
 * Parameters:
 *   file: File pointer (must be opened in binary read mode)
 * 
 * Returns:
 *   0 on success, -1 on error (wrong format or version)
 * 
 * Example:
 *   FILE *f = fopen("data.bin", "rb");
 *   if (serializer_read_header(f) != 0) {
 *       fprintf(stderr, "Invalid file format\n");
 *       fclose(f);
 *       return 1;
 *   }
 *   // ... read data ...
 *   fclose(f);
 */
int serializer_read_header(FILE *file);

/*
 * serializer_get_file_size - Get the size of a file
 * 
 * Returns the size of a file in bytes. Useful for allocating buffers
 * or checking if a file is empty.
 * 
 * Parameters:
 *   filename: Path to the file
 * 
 * Returns:
 *   File size in bytes, or -1 on error
 * 
 * Example:
 *   long size = serializer_get_file_size("data.bin");
 *   if (size > 0) {
 *       printf("File size: %ld bytes\n", size);
 *   }
 */
long serializer_get_file_size(const char *filename);

/*
 * serializer_file_exists - Check if a file exists
 * 
 * Checks whether a file exists and is readable.
 * 
 * Parameters:
 *   filename: Path to the file
 * 
 * Returns:
 *   1 if file exists and is readable, 0 otherwise
 * 
 * Example:
 *   if (serializer_file_exists("data.bin")) {
 *       printf("File exists\n");
 *   }
 */
int serializer_file_exists(const char *filename);

#endif // SERIALIZER_H
