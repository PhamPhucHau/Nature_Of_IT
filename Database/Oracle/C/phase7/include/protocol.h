/*
 * protocol.h - Network Protocol Header
 * 
 * Defines the network protocol for client-server communication.
 * Handles message serialization and deserialization.
 * 
 * Key Concepts:
 * - Protocol: Communication rules
 * - Message: Unit of communication
 * - Serialization: Convert to bytes
 * - Deserialization: Convert from bytes
 * - Message Types: Different message kinds
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Protocol version */
#define PROTOCOL_VERSION 1

/* Message header size */
#define MESSAGE_HEADER_SIZE 16

/* Maximum message size */
#define MAX_MESSAGE_SIZE 65536

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * MessageType - Type of network message
 */
typedef enum {
    MSG_QUERY,          /* Query message */
    MSG_RESULT,         /* Result message */
    MSG_ERROR,          /* Error message */
    MSG_PING,           /* Ping message */
    MSG_PONG,           /* Pong message */
    MSG_CONNECT,        /* Connect message */
    MSG_DISCONNECT      /* Disconnect message */
} MessageType;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * MessageHeader - Header for network message
 * 
 * Fields:
 *   - version: Protocol version
 *   - type: Message type
 *   - length: Message length
 *   - sequence: Message sequence number
 */
typedef struct {
    uint8_t version;                /* Protocol version */
    uint8_t type;                   /* Message type */
    uint16_t length;                /* Message length */
    uint32_t sequence;              /* Sequence number */
} MessageHeader;

/*
 * Message - Network message
 * 
 * Fields:
 *   - header: Message header
 *   - payload: Message payload
 */
typedef struct {
    MessageHeader header;           /* Message header */
    char payload[MAX_MESSAGE_SIZE]; /* Message payload */
} Message;

/*
 * QueryMessage - Query message payload
 * 
 * Fields:
 *   - query_id: Query ID
 *   - query_type: Type of query
 *   - query_text: SQL query text
 */
typedef struct {
    uint32_t query_id;              /* Query ID */
    uint8_t query_type;             /* Query type */
    char query_text[4096];          /* Query text */
} QueryMessage;

/*
 * ResultMessage - Result message payload
 * 
 * Fields:
 *   - query_id: Query ID
 *   - status: Result status
 *   - rows_affected: Rows affected
 *   - result_data: Result data
 */
typedef struct {
    uint32_t query_id;              /* Query ID */
    uint8_t status;                 /* Result status */
    uint32_t rows_affected;         /* Rows affected */
    char result_data[8192];         /* Result data */
} ResultMessage;

/*
 * ErrorMessage - Error message payload
 * 
 * Fields:
 *   - query_id: Query ID
 *   - error_code: Error code
 *   - error_text: Error text
 */
typedef struct {
    uint32_t query_id;              /* Query ID */
    uint32_t error_code;            /* Error code */
    char error_text[256];           /* Error text */
} ErrorMessage;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * protocol_create_message - Create message
 * 
 * Creates a new message with header.
 * 
 * Parameters:
 *   type: Message type
 *   payload: Message payload
 *   length: Payload length
 * 
 * Returns:
 *   Pointer to new Message, or NULL on error
 */
Message* protocol_create_message(MessageType type, const char *payload,
                                 uint16_t length);

/*
 * protocol_destroy_message - Free message
 * 
 * Frees the memory allocated for message.
 * 
 * Parameters:
 *   msg: Pointer to Message
 * 
 * Returns:
 *   void
 */
void protocol_destroy_message(Message *msg);

/*
 * protocol_serialize_message - Serialize message
 * 
 * Serializes message to bytes.
 * 
 * Parameters:
 *   msg: Pointer to Message
 *   buffer: Output buffer
 *   buffer_size: Buffer size
 * 
 * Returns:
 *   Number of bytes written, or -1 on error
 */
int protocol_serialize_message(Message *msg, char *buffer, uint32_t buffer_size);

/*
 * protocol_deserialize_message - Deserialize message
 * 
 * Deserializes message from bytes.
 * 
 * Parameters:
 *   buffer: Input buffer
 *   buffer_size: Buffer size
 *   msg: Pointer to Message
 * 
 * Returns:
 *   Number of bytes read, or -1 on error
 */
int protocol_deserialize_message(const char *buffer, uint32_t buffer_size,
                                 Message *msg);

/*
 * protocol_create_query_message - Create query message
 * 
 * Creates a query message.
 * 
 * Parameters:
 *   query_id: Query ID
 *   query_type: Query type
 *   query_text: SQL query text
 * 
 * Returns:
 *   Pointer to new Message, or NULL on error
 */
Message* protocol_create_query_message(uint32_t query_id, uint8_t query_type,
                                       const char *query_text);

/*
 * protocol_create_result_message - Create result message
 * 
 * Creates a result message.
 * 
 * Parameters:
 *   query_id: Query ID
 *   status: Result status
 *   rows_affected: Rows affected
 *   result_data: Result data
 * 
 * Returns:
 *   Pointer to new Message, or NULL on error
 */
Message* protocol_create_result_message(uint32_t query_id, uint8_t status,
                                        uint32_t rows_affected,
                                        const char *result_data);

/*
 * protocol_create_error_message - Create error message
 * 
 * Creates an error message.
 * 
 * Parameters:
 *   query_id: Query ID
 *   error_code: Error code
 *   error_text: Error text
 * 
 * Returns:
 *   Pointer to new Message, or NULL on error
 */
Message* protocol_create_error_message(uint32_t query_id, uint32_t error_code,
                                       const char *error_text);

/*
 * protocol_get_message_type - Get message type
 * 
 * Returns the type of a message.
 * 
 * Parameters:
 *   msg: Pointer to Message
 * 
 * Returns:
 *   Message type
 */
MessageType protocol_get_message_type(Message *msg);

/*
 * protocol_get_message_length - Get message length
 * 
 * Returns the length of a message.
 * 
 * Parameters:
 *   msg: Pointer to Message
 * 
 * Returns:
 *   Message length
 */
uint16_t protocol_get_message_length(Message *msg);

#endif // PROTOCOL_H
