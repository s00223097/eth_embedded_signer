#ifndef ETH_EMBEDDED_TRANSACTION_H
#define ETH_EMBEDDED_TRANSACTION_H

#include <stdint.h>
#include <stddef.h>
#include "crypto.h"

// Transaction types (legacy, EIP-2930, EIP-1559)
typedef enum {
    ETH_LEGACY_TX = 0,     // Old-school tx (pre-EIP-2718)
    ETH_EIP2930_TX = 1,    // EIP-2930 (access list)
    ETH_EIP1559_TX = 2     // EIP-1559 (priority fee)
} eth_tx_type_t;

// Ethereum transaction aka tx struct
typedef struct {
    eth_tx_type_t tx_type;        // What kind of tx is this? 

    // Stuff every tx has
    uint64_t nonce;               // Sender's nonce
    uint8_t to[20];               // Who's getting it (empty for contract creation)
    uint8_t to_len;               // How long 'to' is (0 = contract creation) - length basically
    uint8_t value[32];            // Amount in wei or whatever denomination
    uint8_t value_len;            // How many bytes in value
    uint8_t *data;                // Calldata or contract code
    size_t data_len;              // Length of data
    uint64_t chain_id;            // Chain ID

    // Only for legacy and EIP-2930
    uint8_t gas_price[32];        // Gas price in wei
    uint8_t gas_price_len;        // How many bytes in gas price

    // for EIP-1559
    uint8_t max_priority_fee[32]; // Max tip per gas (wei)
    uint8_t max_priority_fee_len; // How many bytes in tip
    uint8_t max_fee[32];          // Max fee per gas (wei)
    uint8_t max_fee_len;          // How many bytes in max fee

    // All tx types
    uint64_t gas_limit;           // Max gas for this tx

    // Signature
    uint8_t v;                    // Recovery ID + chain ID
    uint8_t r[32];                // Sig R
    uint8_t s[32];                // Sig S
} eth_transaction_t;

/**
 * @brief Initialise a transaction structure
 * 
 * @param tx Pointer to transaction structure
 * @param tx_type Transaction type (legacy, EIP-2930, or EIP-1559)
 * @return 0 on success, non-zero on error
 */
int eth_tx_init(eth_transaction_t *tx, eth_tx_type_t tx_type);

/**
 * @brief RLP encode a transaction (unsigned)
 * 
 * @param tx Pointer to transaction structure
 * @param buffer Output buffer for encoded transaction
 * @param buffer_size Size of output buffer
 * @param output_size Pointer to variable to store size of encoded transaction
 * @return 0 on success, non-zero on error
 */
int eth_tx_encode(const eth_transaction_t *tx, uint8_t *buffer, size_t buffer_size, size_t *output_size);

/**
 * @brief Hash a transaction for signing
 * 
 * @param tx Pointer to transaction structure
 * @param hash Output hash
 * @return 0 on success, non-zero on error
 */
int eth_tx_hash(const eth_transaction_t *tx, eth_hash_t *hash);

/**
 * @brief Sign a transaction with a private key
 * 
 * @param tx Pointer to transaction structure
 * @param private_key Private key to sign with
 * @return 0 on success, non-zero on error
 */
int eth_tx_sign(eth_transaction_t *tx, const eth_private_key_t *private_key);

/**
 * @brief RLP encode a signed transaction
 * 
 * @param tx Pointer to transaction structure
 * @param buffer Output buffer for encoded transaction
 * @param buffer_size Size of output buffer
 * @param output_size Pointer to variable to store size of encoded transaction
 * @return 0 on success, non-zero on error
 */
int eth_tx_encode_signed(const eth_transaction_t *tx, uint8_t *buffer, size_t buffer_size, size_t *output_size);

#endif /* ETH_EMBEDDED_TRANSACTION_H */ 