/*
 * protocol.c - Network Protocol Implementation
 * 
 * Defines the network protocol for client-server communication.
 * Handles message serialization and deserialization.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "protocol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * PROTOCOL FUNCTIONS
 * ============================================================================ */

static uint32_t next_sequence = 1;

Message* protocol_create_message(MessageType type, const char *payload,
                                 uint16_t length) {
    Message *msg = (Message *)malloc(sizeof(Message));
    if (!msg) return NULL;
    
    memset(msg, 0, sizeof(Message));
    msg->header.version = PROTOCOL_VERSION;
    msg->header.type = type;
    msg->header.length = length;
    msg->header.sequence = next_sequence++;
    
    if (payload && length > 0) {
        memcpy(msg->payload, payload, length < MAX_MESSAGE_SIZE ? length : MAX_MESSAGE_SIZE);
    }
    
    return msg;
}

void protocol_destroy_message(Message *msg) {
    if (!msg) return;
    free(msg);
}

int protocol_serialize_message(Message *msg, char *buffer, uint32_t buffer_size) {
    if (!msg || !buffer) return -1;
    
    uint32_t total_size = MESSAGE_HEADER_SIZE + msg->header.length;
    if (total_size > buffer_size) return -1;
    
    /* Serialize header */
    memcpy(buffer, &msg->header.version, 1);
    memcpy(buffer + 1, &msg->header.type, 1);
    memcpy(buffer + 2, &msg->header.length, 2);
    memcpy(buffer + 4, &msg->header.sequence, 4);
    
    /* Serialize payload */
    if (msg->header.length > 0) {
        memcpy(buffer + MESSAGE_HEADER_SIZE, msg->payload, msg->header.length);
    }
    
    return total_size;
}

int protocol_deserialize_message(const char *buffer, uint32_t buffer_size,
                                 Message *msg) {
    if (!buffer || !msg) return -1;
    
    if (buffer_size < MESSAGE_HEADER_SIZE) return -1;
    
    /* Deserialize header */
    memcpy(&msg->header.version, buffer, 1);
    memcpy(&msg->header.type, buffer + 1, 1);
    memcpy(&msg->header.length, buffer + 2, 2);
    memcpy(&msg->header.sequence, buffer + 4, 4);
    
    uint32_t total_size = MESSAGE_HEADER_SIZE + msg->header.length;
    if (total_size > buffer_size) return -1;
    
    /* Deserialize payload */
    if (msg->header.length > 0) {
        memcpy(msg->payload, buffer + MESSAGE_HEADER_SIZE, msg->header.length);
    }
    
    return total_size;
}

Message* protocol_create_query_message(uint32_t query_id, uint8_t query_type,
                                       const char *query_text) {
    QueryMessage qmsg;
    memset(&qmsg, 0, sizeof(QueryMessage));
    
    qmsg.query_id = query_id;
    qmsg.query_type = query_type;
    if (query_text) {
        strncpy(qmsg.query_text, query_text, sizeof(qmsg.query_text) - 1);
    }
    
    return protocol_create_message(MSG_QUERY, (const char *)&qmsg, sizeof(QueryMessage));
}

Message* protocol_create_result_message(uint32_t query_id, uint8_t status,
                                        uint32_t rows_affected,
                                        const char *result_data) {
    ResultMessage rmsg;
    memset(&rmsg, 0, sizeof(ResultMessage));
    
    rmsg.query_id = query_id;
    rmsg.status = status;
    rmsg.rows_affected = rows_affected;
    if (result_data) {
        strncpy(rmsg.result_data, result_data, sizeof(rmsg.result_data) - 1);
    }
    
    return protocol_create_message(MSG_RESULT, (const char *)&rmsg, sizeof(ResultMessage));
}

Message* protocol_create_error_message(uint32_t query_id, uint32_t error_code,
                                       const char *error_text) {
    ErrorMessage emsg;
    memset(&emsg, 0, sizeof(ErrorMessage));
    
    emsg.query_id = query_id;
    emsg.error_code = error_code;
    if (error_text) {
        strncpy(emsg.error_text, error_text, sizeof(emsg.error_text) - 1);
    }
    
    return protocol_create_message(MSG_ERROR, (const char *)&emsg, sizeof(ErrorMessage));
}

MessageType protocol_get_message_type(Message *msg) {
    if (!msg) return MSG_QUERY;
    return (MessageType)msg->header.type;
}

uint16_t protocol_get_message_length(Message *msg) {
    if (!msg) return 0;
    return msg->header.length;
}
