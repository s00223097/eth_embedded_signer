#ifndef ETH_EMBEDDED_CRYPTO_H
#define ETH_EMBEDDED_CRYPTO_H

#include <stdint.h>
#include <stddef.h>

/* Type definitions */
typedef uint8_t eth_byte_t;
typedef struct {
    eth_byte_t data[32];
} eth_hash_t;

typedef struct {
    eth_byte_t data[64];
} eth_signature_t;

typedef struct {
    eth_byte_t data[32];
} eth_private_key_t;

typedef struct {
    eth_byte_t data[64];
} eth_public_key_t;

typedef struct {
    eth_byte_t data[20];
} eth_address_t;

/* Function declarations */

/**
 * @brief Compute Keccak-256 hash of input data
 * 
 * @param input Pointer to input data
 * @param input_len Length of input data in bytes
 * @param output Pointer to output hash (32 bytes)
 * @return 0 on success, non-zero on error
 */
int eth_keccak256(const eth_byte_t *input, size_t input_len, eth_hash_t *output);

/**
 * @brief Sign a message hash with a private key using ECDSA
 * 
 * @param msg_hash Hash of the message to sign (32 bytes)
 * @param private_key Private key to sign with (32 bytes)
 * @param signature Output signature (64 bytes: r and s concatenated)
 * @return 0 on success, non-zero on error
 */
int eth_sign(const eth_hash_t *msg_hash, const eth_private_key_t *private_key, eth_signature_t *signature);

/**
 * @brief Recover public key from signature and message hash
 * 
 * @param signature Signature (64 bytes: r and s concatenated)
 * @param msg_hash Hash of the signed message (32 bytes)
 * @param recovery_id Recovery ID (0 or 1)
 * @param public_key Output public key (64 bytes: x and y coordinates concatenated)
 * @return 0 on success, non-zero on error
 */
int eth_recover_public_key(const eth_signature_t *signature, const eth_hash_t *msg_hash, 
                          uint8_t recovery_id, eth_public_key_t *public_key);

/**
 * @brief Derive Ethereum address from public key
 * 
 * @param public_key Public key (64 bytes: x and y coordinates concatenated)
 * @param address Output address (20 bytes)
 * @return 0 on success, non-zero on error
 */
int eth_public_key_to_address(const eth_public_key_t *public_key, eth_address_t *address);

#endif /* ETH_EMBEDDED_CRYPTO_H - look into this*/ 