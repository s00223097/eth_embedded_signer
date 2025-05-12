#ifndef ETH_EMBEDDED_RLP_H
#define ETH_EMBEDDED_RLP_H

#include <stdint.h>
#include <stddef.h>

/* RLP encoding types */
typedef enum {
    RLP_DATA_ITEM,  /* Single data item */
    RLP_LIST        /* List of items */
} rlp_type_t;

/* RLP encoder context */
typedef struct {
    uint8_t *buffer;      /* Output buffer */
    size_t buffer_size;   /* Size of output buffer */
    size_t length;        /* Current length of encoded data */
} rlp_encoder_t;

/**
 * @brief Initialize an RLP encoder
 * 
 * @param encoder Pointer to encoder context
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return 0 on success, non-zero on error
 */
int rlp_encoder_init(rlp_encoder_t *encoder, uint8_t *buffer, size_t buffer_size);

/**
 * @brief Begin an RLP list
 * 
 * @param encoder Pointer to encoder context
 * @param marker Position marker to store position (for later finalization)
 * @return 0 on success, non-zero on error
 */
int rlp_begin_list(rlp_encoder_t *encoder, size_t *marker);

/**
 * @brief End an RLP list (finalize)
 * 
 * @param encoder Pointer to encoder context
 * @param marker Position marker from begin_list
 * @return 0 on success, non-zero on error
 */
int rlp_end_list(rlp_encoder_t *encoder, size_t marker);

/**
 * @brief Encode a single byte
 * 
 * @param encoder Pointer to encoder context
 * @param value Byte to encode
 * @return 0 on success, non-zero on error
 */
int rlp_encode_byte(rlp_encoder_t *encoder, uint8_t value);

/**
 * @brief Encode a byte array
 * 
 * @param encoder Pointer to encoder context
 * @param data Pointer to data
 * @param length Length of data
 * @return 0 on success, non-zero on error
 */
int rlp_encode_bytes(rlp_encoder_t *encoder, const uint8_t *data, size_t length);

/**
 * @brief Encode an unsigned integer (big endian)
 * 
 * @param encoder Pointer to encoder context
 * @param value Integer to encode
 * @return 0 on success, non-zero on error
 */
int rlp_encode_uint(rlp_encoder_t *encoder, uint64_t value);

/**
 * @brief Get the current length of encoded data
 * 
 * @param encoder Pointer to encoder context
 * @return Current length of encoded data
 */
size_t rlp_get_length(const rlp_encoder_t *encoder);

#endif /* ETH_EMBEDDED_RLP_H */ 