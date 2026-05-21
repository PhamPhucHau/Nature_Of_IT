#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdio.h>    // For FILE
#include <stdint.h>   // For uint32_t, uint8_t
#include <stddef.h>   // For size_t

#define SERIALIZER_MAGIC 0x4B5642  // "KVB" in hex

/* Current serialization format version */
#define SERIALIZER_VERSION 1
int serializer_write_uint32(FILE *file, uint32_t value);
int serializer_read_uint32(FILE *file, uint32_t *value); 
int serializer_write_string(FILE *file, const char *str);
int serializer_read_string(FILE *file, char **str);
int serializer_write_bytes(FILE *file, const void *data, size_t size);
int serializer_read_bytes(FILE *file, void *data, size_t size);
int serializer_write_header(FILE *file);
int serializer_read_header(FILE *file);
long serializer_get_file_size(const char *filename);
int serializer_file_exists(const char *filename);

#endif // SERIALIZER_H