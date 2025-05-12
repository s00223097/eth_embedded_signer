#include <string.h>
#include "../include/transaction.h"
#include "../include/rlp.h"

/* Error codes */
#define TX_ERROR_NONE           0
#define TX_ERROR_INVALID       -1
#define TX_ERROR_BUFFER_SMALL  -2
#define TX_ERROR_UNSUPPORTED   -3

/* Initialize a transaction structure */
int eth_tx_init(eth_transaction_t *tx, eth_tx_type_t tx_type) {
    if (!tx) {
        return TX_ERROR_INVALID;
    }
    
    /* Clear the structure */
    memset(tx, 0, sizeof(eth_transaction_t));
    
    /* Set transaction type */
    tx->tx_type = tx_type;
    
    return TX_ERROR_NONE;
}

/* Helper function to encode a transaction field as bytes */
static int encode_tx_field_bytes(rlp_encoder_t *encoder, const uint8_t *data, uint8_t length) {
    return rlp_encode_bytes(encoder, data, length);
}

/* Helper function to encode a transaction field as uint */
static int encode_tx_field_uint(rlp_encoder_t *encoder, uint64_t value) {
    return rlp_encode_uint(encoder, value);
}

/* Helper function to encode transaction data field */
static int encode_tx_data(rlp_encoder_t *encoder, const uint8_t *data, size_t length) {
    if (!data && length > 0) {
        return TX_ERROR_INVALID;
    }
    
    return rlp_encode_bytes(encoder, data, length);
}

/* Helper function to encode a transaction based on type */
static int encode_tx_by_type(const eth_transaction_t *tx, rlp_encoder_t *encoder, bool include_signature) {
    int result;
    size_t list_marker;
    
    /* Start RLP list */
    result = rlp_begin_list(encoder, &list_marker);
    if (result != 0) {
        return result;
    }
    
    /* Encode transaction based on type */
    if (tx->tx_type == ETH_LEGACY_TX) {
        /* Legacy transaction fields */
        
        /* 1. Nonce */
        result = encode_tx_field_uint(encoder, tx->nonce);
        if (result != 0) return result;
        
        /* 2. Gas price */
        result = encode_tx_field_bytes(encoder, tx->gas_price, tx->gas_price_len);
        if (result != 0) return result;
        
        /* 3. Gas limit */
        result = encode_tx_field_uint(encoder, tx->gas_limit);
        if (result != 0) return result;
        
        /* 4. To address */
        if (tx->to_len == 0) {
            /* Contract creation: empty address */
            result = rlp_encode_bytes(encoder, NULL, 0);
        } else {
            result = rlp_encode_bytes(encoder, tx->to, tx->to_len);
        }
        if (result != 0) return result;
        
        /* 5. Value */
        result = encode_tx_field_bytes(encoder, tx->value, tx->value_len);
        if (result != 0) return result;
        
        /* 6. Data */
        result = encode_tx_data(encoder, tx->data, tx->data_len);
        if (result != 0) return result;
        
        if (include_signature) {
            /* 7. V */
            result = rlp_encode_byte(encoder, tx->v);
            if (result != 0) return result;
            
            /* 8. R */
            result = rlp_encode_bytes(encoder, tx->r, 32);
            if (result != 0) return result;
            
            /* 9. S */
            result = rlp_encode_bytes(encoder, tx->s, 32);
            if (result != 0) return result;
        } else {
            /* For EIP-155 replay protection: v = chainId, r = 0, s = 0 */
            result = rlp_encode_uint(encoder, tx->chain_id);
            if (result != 0) return result;
            
            /* Empty R */
            result = rlp_encode_bytes(encoder, NULL, 0);
            if (result != 0) return result;
            
            /* Empty S */
            result = rlp_encode_bytes(encoder, NULL, 0);
            if (result != 0) return result;
        }
    } else if (tx->tx_type == ETH_EIP2930_TX) {
        /* EIP-2930 transaction fields */
        
        /* 1. Chain ID */
        result = encode_tx_field_uint(encoder, tx->chain_id);
        if (result != 0) return result;
        
        /* 2. Nonce */
        result = encode_tx_field_uint(encoder, tx->nonce);
        if (result != 0) return result;
        
        /* 3. Gas price */
        result = encode_tx_field_bytes(encoder, tx->gas_price, tx->gas_price_len);
        if (result != 0) return result;
        
        /* 4. Gas limit */
        result = encode_tx_field_uint(encoder, tx->gas_limit);
        if (result != 0) return result;
        
        /* 5. To address */
        if (tx->to_len == 0) {
            /* Contract creation: empty address */
            result = rlp_encode_bytes(encoder, NULL, 0);
        } else {
            result = rlp_encode_bytes(encoder, tx->to, tx->to_len);
        }
        if (result != 0) return result;
        
        /* 6. Value */
        result = encode_tx_field_bytes(encoder, tx->value, tx->value_len);
        if (result != 0) return result;
        
        /* 7. Data */
        result = encode_tx_data(encoder, tx->data, tx->data_len);
        if (result != 0) return result;
        
        /* 8. Access list (empty for this implementation) */
        size_t access_list_marker;
        result = rlp_begin_list(encoder, &access_list_marker);
        if (result != 0) return result;
        
        result = rlp_end_list(encoder, access_list_marker);
        if (result != 0) return result;
        
        if (include_signature) {
            /* 9. V (just recovery ID 0/1 for EIP-2930) */
            result = rlp_encode_byte(encoder, tx->v);
            if (result != 0) return result;
            
            /* 10. R */
            result = rlp_encode_bytes(encoder, tx->r, 32);
            if (result != 0) return result;
            
            /* 11. S */
            result = rlp_encode_bytes(encoder, tx->s, 32);
            if (result != 0) return result;
        }
    } else if (tx->tx_type == ETH_EIP1559_TX) {
        /* EIP-1559 transaction fields */
        
        /* 1. Chain ID */
        result = encode_tx_field_uint(encoder, tx->chain_id);
        if (result != 0) return result;
        
        /* 2. Nonce */
        result = encode_tx_field_uint(encoder, tx->nonce);
        if (result != 0) return result;
        
        /* 3. Max priority fee per gas */
        result = encode_tx_field_bytes(encoder, tx->max_priority_fee, tx->max_priority_fee_len);
        if (result != 0) return result;
        
        /* 4. Max fee per gas */
        result = encode_tx_field_bytes(encoder, tx->max_fee, tx->max_fee_len);
        if (result != 0) return result;
        
        /* 5. Gas limit */
        result = encode_tx_field_uint(encoder, tx->gas_limit);
        if (result != 0) return result;
        
        /* 6. To address */
        if (tx->to_len == 0) {
            /* Contract creation: empty address */
            result = rlp_encode_bytes(encoder, NULL, 0);
        } else {
            result = rlp_encode_bytes(encoder, tx->to, tx->to_len);
        }
        if (result != 0) return result;
        
        /* 7. Value */
        result = encode_tx_field_bytes(encoder, tx->value, tx->value_len);
        if (result != 0) return result;
        
        /* 8. Data */
        result = encode_tx_data(encoder, tx->data, tx->data_len);
        if (result != 0) return result;
        
        /* 9. Access list (empty for this implementation) */
        size_t access_list_marker;
        result = rlp_begin_list(encoder, &access_list_marker);
        if (result != 0) return result;
        
        result = rlp_end_list(encoder, access_list_marker);
        if (result != 0) return result;
        
        if (include_signature) {
            /* 10. V (just recovery ID 0/1 for EIP-1559) */
            result = rlp_encode_byte(encoder, tx->v);
            if (result != 0) return result;
            
            /* 11. R */
            result = rlp_encode_bytes(encoder, tx->r, 32);
            if (result != 0) return result;
            
            /* 12. S */
            result = rlp_encode_bytes(encoder, tx->s, 32);
            if (result != 0) return result;
        }
    } else {
        return TX_ERROR_UNSUPPORTED;
    }
    
    /* Finalize the list */
    result = rlp_end_list(encoder, list_marker);
    
    return result;
}

/* RLP encode a transaction (unsigned) */
int eth_tx_encode(const eth_transaction_t *tx, uint8_t *buffer, size_t buffer_size, size_t *output_size) {
    if (!tx || !buffer || buffer_size == 0 || !output_size) {
        return TX_ERROR_INVALID;
    }
    
    rlp_encoder_t encoder;
    int result;
    
    /* Initialize the RLP encoder */
    result = rlp_encoder_init(&encoder, buffer, buffer_size);
    if (result != 0) {
        return result;
    }
    
    /* For EIP-2930 and EIP-1559, we need to prefix with transaction type */
    if (tx->tx_type == ETH_EIP2930_TX) {
        result = rlp_encode_byte(&encoder, 0x01);
        if (result != 0) {
            return result;
        }
    } else if (tx->tx_type == ETH_EIP1559_TX) {
        result = rlp_encode_byte(&encoder, 0x02);
        if (result != 0) {
            return result;
        }
    }
    
    /* Encode the transaction (unsigned) */
    result = encode_tx_by_type(tx, &encoder, false);
    if (result != 0) {
        return result;
    }
    
    /* Get the encoded size */
    *output_size = rlp_get_length(&encoder);
    
    return TX_ERROR_NONE;
}

/* Hash a transaction for signing */
int eth_tx_hash(const eth_transaction_t *tx, eth_hash_t *hash) {
    if (!tx || !hash) {
        return TX_ERROR_INVALID;
    }
    
    /* Encode the transaction for hashing */
    uint8_t buffer[1024]; /* Buffer for encoded transaction */
    size_t encoded_size;
    
    /* Encode the transaction */
    int result = eth_tx_encode(tx, buffer, sizeof(buffer), &encoded_size);
    if (result != 0) {
        return result;
    }
    
    /* Hash the encoded transaction */
    return eth_keccak256(buffer, encoded_size, hash);
}

/* Sign a transaction with a private key */
int eth_tx_sign(eth_transaction_t *tx, const eth_private_key_t *private_key) {
    if (!tx || !private_key) {
        return TX_ERROR_INVALID;
    }
    
    /* Hash the transaction for signing */
    eth_hash_t hash;
    int result = eth_tx_hash(tx, &hash);
    if (result != 0) {
        return result;
    }
    
    /* Sign the hash */
    eth_signature_t signature;
    result = eth_sign(&hash, private_key, &signature);
    if (result != 0) {
        return result;
    }
    
    /* Copy R and S components */
    memcpy(tx->r, signature.data, 32);
    memcpy(tx->s, signature.data + 32, 32);
    
    /* Set V (recovery ID) - for simplicity, use 0 here */
    /* In a real implementation, you would determine the correct recovery ID */
    if (tx->tx_type == ETH_LEGACY_TX) {
        /* For EIP-155, V = 35/36 + chainId*2 */
        tx->v = 35 + (tx->chain_id * 2);
    } else {
        /* For EIP-2930/1559, V is just the recovery ID (0/1) */
        tx->v = 0;
    }
    
    return TX_ERROR_NONE;
}

/* RLP encode a signed transaction */
int eth_tx_encode_signed(const eth_transaction_t *tx, uint8_t *buffer, size_t buffer_size, size_t *output_size) {
    if (!tx || !buffer || buffer_size == 0 || !output_size) {
        return TX_ERROR_INVALID;
    }
    
    rlp_encoder_t encoder;
    int result;
    
    /* Initialize the RLP encoder */
    result = rlp_encoder_init(&encoder, buffer, buffer_size);
    if (result != 0) {
        return result;
    }
    
    /* For EIP-2930 and EIP-1559, we need to prefix with transaction type */
    if (tx->tx_type == ETH_EIP2930_TX) {
        result = rlp_encode_byte(&encoder, 0x01);
        if (result != 0) {
            return result;
        }
    } else if (tx->tx_type == ETH_EIP1559_TX) {
        result = rlp_encode_byte(&encoder, 0x02);
        if (result != 0) {
            return result;
        }
    }
    
    /* Encode the transaction (with signature) */
    result = encode_tx_by_type(tx, &encoder, true);
    if (result != 0) {
        return result;
    }
    
    /* Get the encoded size */
    *output_size = rlp_get_length(&encoder);
    
    return TX_ERROR_NONE;
} 