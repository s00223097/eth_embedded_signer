#include <string.h>
#include "../include/rlp.h"

/* RLP encoding prefix */
#define RLP_SINGLE_BYTE_PREFIX     0x80
#define RLP_SHORT_STRING_PREFIX    0x80
#define RLP_LONG_STRING_PREFIX     0xb7
#define RLP_SHORT_LIST_PREFIX      0xc0
#define RLP_LONG_LIST_PREFIX       0xf7

/* Error codes */
#define RLP_ERROR_NONE             0
#define RLP_ERROR_BUFFER_OVERFLOW -1
#define RLP_ERROR_INVALID_PARAM   -2

/* Private functions */

/* Insert bytes into buffer with bounds checking */
static int rlp_insert(rlp_encoder_t *encoder, const uint8_t *data, size_t length) {
    if (encoder->length + length > encoder->buffer_size) {
        return RLP_ERROR_BUFFER_OVERFLOW;
    }
    
    memcpy(encoder->buffer + encoder->length, data, length);
    encoder->length += length;
    
    return RLP_ERROR_NONE;
}

/* Insert a single byte into buffer with bounds checking */
static int rlp_insert_byte(rlp_encoder_t *encoder, uint8_t byte) {
    if (encoder->length + 1 > encoder->buffer_size) {
        return RLP_ERROR_BUFFER_OVERFLOW;
    }
    
    encoder->buffer[encoder->length++] = byte;
    
    return RLP_ERROR_NONE;
}

/* Write length prefix for string/list with bounds checking */
static int rlp_write_length_prefix(rlp_encoder_t *encoder, size_t length, uint8_t base_prefix) {
    if (length < 56) { 
        /* Short string/list: single byte prefix + length */
        return rlp_insert_byte(encoder, base_prefix + length);
    } else {
        /* Long string/list: prefix + length of length + length bytes */
        uint8_t length_bytes[8];
        size_t length_size = 0;
        size_t temp = length;
        
        /* Encode length in big-endian format */
        do {
            length_bytes[length_size++] = temp & 0xFF;
            temp >>= 8;
        } while (temp > 0);
        
        /* Reverse to get big-endian representation */
        for (size_t i = 0; i < length_size / 2; i++) {
            uint8_t temp = length_bytes[i];
            length_bytes[i] = length_bytes[length_size - i - 1];
            length_bytes[length_size - i - 1] = temp;
        }
        
        /* Write prefix + length of length */
        if (rlp_insert_byte(encoder, base_prefix + 55 + length_size) != RLP_ERROR_NONE) {
            return RLP_ERROR_BUFFER_OVERFLOW;
        }
        
        /* Write length bytes */
        return rlp_insert(encoder, length_bytes, length_size);
    }
}

/* Public API implementation */

int rlp_encoder_init(rlp_encoder_t *encoder, uint8_t *buffer, size_t buffer_size) {
    if (!encoder || !buffer || buffer_size == 0) {
        return RLP_ERROR_INVALID_PARAM;
    }
    
    encoder->buffer = buffer;
    encoder->buffer_size = buffer_size;
    encoder->length = 0;
    
    return RLP_ERROR_NONE;
}

int rlp_begin_list(rlp_encoder_t *encoder, size_t *marker) {
    if (!encoder || !marker) {
        return RLP_ERROR_INVALID_PARAM;
    }
    
    /* Store current position for later update */
    *marker = encoder->length;
    
    /* Reserve space for list prefix (assuming short list but Idk) */
    return rlp_insert_byte(encoder, RLP_SHORT_LIST_PREFIX);
}

int rlp_end_list(rlp_encoder_t *encoder, size_t marker) {
    if (!encoder || marker >= encoder->length) {
        return RLP_ERROR_INVALID_PARAM;
    }
    
    size_t list_length = encoder->length - marker - 1;
    
    if (list_length < 56) {
        /* Short list, just update the prefix */
        encoder->buffer[marker] = RLP_SHORT_LIST_PREFIX + list_length;
    } else {
        /* Long list, we need to shift data and insert length bytes */
        uint8_t length_bytes[8];
        size_t length_size = 0;
        size_t temp = list_length;
        
        /* Encode length in big-endian format */
        do {
            length_bytes[length_size++] = temp & 0xFF;
            temp >>= 8;
        } while (temp > 0);
        
        /* Reverse to get big-endian representation */
        for (size_t i = 0; i < length_size / 2; i++) {
            uint8_t temp = length_bytes[i];
            length_bytes[i] = length_bytes[length_size - i - 1];
            length_bytes[length_size - i - 1] = temp;
        }
        
        /* Check if we have enough space for the larger prefix */
        if (encoder->length + length_size > encoder->buffer_size) {
            return RLP_ERROR_BUFFER_OVERFLOW;
        }
        
        /* Shift data to make room for length bytes */
        memmove(encoder->buffer + marker + 1 + length_size, 
                encoder->buffer + marker + 1, 
                list_length);
        
        /* Update prefix and insert length bytes */
        encoder->buffer[marker] = RLP_LONG_LIST_PREFIX + length_size;
        memcpy(encoder->buffer + marker + 1, length_bytes, length_size);
        
        /* Update total length */
        encoder->length += length_size;
    }
    
    return RLP_ERROR_NONE;
}

int rlp_encode_byte(rlp_encoder_t *encoder, uint8_t value) {
    if (!encoder) {
        return RLP_ERROR_INVALID_PARAM;
    }
    
    if (value == 0) {
        /* Encode 0 as 0x80 */
        return rlp_insert_byte(encoder, RLP_SINGLE_BYTE_PREFIX);
    } else if (value < 0x80) {
        /* Encode values < 0x80 as themselves */
        return rlp_insert_byte(encoder, value);
    } else {
        /* Encode values >= 0x80 as single-byte strings */
        if (rlp_insert_byte(encoder, RLP_SHORT_STRING_PREFIX + 1) != RLP_ERROR_NONE) {
            return RLP_ERROR_BUFFER_OVERFLOW;
        }
        return rlp_insert_byte(encoder, value);
    }
}

int rlp_encode_bytes(rlp_encoder_t *encoder, const uint8_t *data, size_t length) {
    if (!encoder || (!data && length > 0)) {
        return RLP_ERROR_INVALID_PARAM;
    }
    
    if (length == 0) {
        /* Empty string encoded as 0x80 */
        return rlp_insert_byte(encoder, RLP_SINGLE_BYTE_PREFIX);
    } else if (length == 1 && data[0] < 0x80) {
        /* Single byte < 0x80 encoded as itself */
        return rlp_insert_byte(encoder, data[0]);
    } else {
        /* String encoded with length prefix */
        int result = rlp_write_length_prefix(encoder, length, RLP_SHORT_STRING_PREFIX);
        if (result != RLP_ERROR_NONE) {
            return result;
        }
        
        return rlp_insert(encoder, data, length);
    }
}

int rlp_encode_uint(rlp_encoder_t *encoder, uint64_t value) {
    if (!encoder) {
        return RLP_ERROR_INVALID_PARAM;
    }
    
    if (value == 0) {
        /* Zero encoded as 0x80 */
        return rlp_insert_byte(encoder, RLP_SINGLE_BYTE_PREFIX);
    } else {
        /* Convert to big-endian byte array */
        uint8_t bytes[8];
        size_t length = 0;
        uint64_t temp = value;
        
        do {
            bytes[length++] = temp & 0xFF;
            temp >>= 8;
        } while (temp > 0);
        
        /* Reverse to get big-endian representation */
        uint8_t encoded[8];
        for (size_t i = 0; i < length; i++) {
            encoded[i] = bytes[length - i - 1];
        }
        
        /* Encode as byte array */
        return rlp_encode_bytes(encoder, encoded, length);
    }
}

size_t rlp_get_length(const rlp_encoder_t *encoder) {
    return encoder ? encoder->length : 0;
} 