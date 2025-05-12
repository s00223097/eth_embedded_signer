#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../include/crypto.h"
#include "../include/transaction.h"
#include "../include/rlp.h"

/* Print a byte array as hex */
static void print_hex(const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02x", data[i]);
    }
}

/* Make a test private key (never use this for real ETH!) */
static void generate_sample_private_key(eth_private_key_t *private_key) {
    /* This is a fixed test key, super insecure, just for demo */
    const uint8_t test_key[32] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    
    memcpy(private_key->data, test_key, 32);
}

/* Make a sample EIP-1559 transaction (for demo) */
static void create_sample_eip1559_transaction(eth_transaction_t *tx) {
    eth_tx_init(tx, ETH_EIP1559_TX);
    tx->chain_id = 1; /* Ethereum mainnet */
    tx->nonce = 42;   /* Just a random nonce */
    
    /* Set gas parameters */
    uint8_t max_priority_fee[1] = { 0x01 }; /* 1 gwei */
    tx->max_priority_fee_len = 1;
    memcpy(tx->max_priority_fee, max_priority_fee, tx->max_priority_fee_len);
    
    uint8_t max_fee[1] = { 0x05 }; /* 5 gwei */
    tx->max_fee_len = 1;
    memcpy(tx->max_fee, max_fee, tx->max_fee_len);
    
    tx->gas_limit = 21000; /* Standard ETH transfer gas */
    
    /* Set recipient address (example address) */
    uint8_t to[20] = {
        0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x12, 0x34,
        0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78
    };
    tx->to_len = 20;
    memcpy(tx->to, to, tx->to_len);
    
    /* Set transaction value (1 ETH = 10^18 wei) */
    uint8_t value[4] = { 0x0d, 0xe0, 0xb6, 0xb3 }; /* 0.0001 ETH, just for fun */
    tx->value_len = 4;
    memcpy(tx->value, value, tx->value_len);
    
    /* No data for a simple ETH transfer */
    tx->data = NULL;
    tx->data_len = 0;
}

/* Make a sample legacy transaction (for demo) */
static void create_sample_legacy_transaction(eth_transaction_t *tx) {
    eth_tx_init(tx, ETH_LEGACY_TX);
    tx->chain_id = 1; /* Ethereum mainnet */
    tx->nonce = 42;   /* Just a random nonce */
    
    /* Set gas parameters */
    uint8_t gas_price[1] = { 0x03 }; /* 3 gwei */
    tx->gas_price_len = 1;
    memcpy(tx->gas_price, gas_price, tx->gas_price_len);
    
    tx->gas_limit = 21000; /* Standard ETH transfer gas */
    
    /* Set recipient address (example address) */
    uint8_t to[20] = {
        0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x12, 0x34,
        0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78
    };
    tx->to_len = 20;
    memcpy(tx->to, to, tx->to_len);
    
    /* Set transaction value (1 ETH = 10^18 wei) */
    uint8_t value[4] = { 0x0d, 0xe0, 0xb6, 0xb3 }; /* 0.0001 ETH, just for fun */
    tx->value_len = 4;
    memcpy(tx->value, value, tx->value_len);
    
    /* No data for a simple ETH transfer */
    tx->data = NULL;
    tx->data_len = 0;
}

/* Make a contract call tx (EIP-1559 style, for demo) */
static void create_sample_contract_interaction(eth_transaction_t *tx) {
    eth_tx_init(tx, ETH_EIP1559_TX);
    tx->chain_id = 1; /* Ethereum mainnet */
    tx->nonce = 42;   /* Just a random nonce */
    
    /* Set gas parameters */
    uint8_t max_priority_fee[1] = { 0x01 }; /* 1 gwei */
    tx->max_priority_fee_len = 1;
    memcpy(tx->max_priority_fee, max_priority_fee, tx->max_priority_fee_len);
    
    uint8_t max_fee[1] = { 0x05 }; /* 5 gwei */
    tx->max_fee_len = 1;
    memcpy(tx->max_fee, max_fee, tx->max_fee_len);
    
    tx->gas_limit = 150000; /* More gas for contract call */
    
    /* Set contract address (example address) */
    uint8_t to[20] = {
        0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad,
        0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef
    };
    tx->to_len = 20;
    memcpy(tx->to, to, tx->to_len);
    
    /* Value: 0 ETH for this tx */
    uint8_t value[1] = { 0x00 };
    tx->value_len = 1;
    memcpy(tx->value, value, tx->value_len);
    
    /* Example function call: transfer(address,uint256) with address and value arguments */
    /* Function signature: transfer(address,uint256) = 0xa9059cbb */
    /* Address parameter: 0x1234567890abcdef1234567890abcdef12345678 [//TODO: make this a variable] */
    /* Value parameter: 1000000000000000000 (1 ETH) */
    static uint8_t data[] = {
        0xa9, 0x05, 0x9c, 0xbb, /* Function signature */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Padding */
        0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, /* Address */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Padding */
        0x0d, 0xe0, 0xb6, 0xb3, 0xa7, 0x64, 0x00, 0x00 /* Value: 1 ETH (0x0de0b6b3a7640000) */
    };
    
    tx->data = data;
    tx->data_len = sizeof(data);
}

/* Demo function to showcase crypto operations */
static void demo_crypto_operations(void) {
    printf("\n--- Crypto Operations Demo ---\n");
    
    /* Generate sample private key */
    eth_private_key_t private_key;
    generate_sample_private_key(&private_key);
    
    printf("Private key: ");
    print_hex(private_key.data, 32);
    printf("\n");
    
    /* Hash some sample data */
    const char *message = "Example message for hashing";
    eth_hash_t hash;
    
    int result = eth_keccak256((const eth_byte_t *)message, strlen(message), &hash);
    
    printf("Message hash: ");
    print_hex(hash.data, 32);
    printf("\n");
    
    /* Sign the hash */
    eth_signature_t signature;
    result = eth_sign(&hash, &private_key, &signature);
    
    printf("Signature R: ");
    print_hex(signature.data, 32);
    printf("\n");
    
    printf("Signature S: ");
    print_hex(signature.data + 32, 32);
    printf("\n");
    
    /* Recover public key from signature */
    eth_public_key_t public_key;
    result = eth_recover_public_key(&signature, &hash, 0, &public_key);
    
    printf("Public key X: ");
    print_hex(public_key.data, 32);
    printf("\n");
    
    printf("Public key Y: ");
    print_hex(public_key.data + 32, 32);
    printf("\n");
    
    /* Derive Ethereum address from public key */
    eth_address_t address;
    result = eth_public_key_to_address(&public_key, &address);
    
    printf("Ethereum address: 0x");
    print_hex(address.data, 20);
    printf("\n");
}

/* Demo function to showcase transaction signing */
static void demo_transaction_signing(void) {
    printf("\n--- Transaction Signing Demo ---\n");
    
    /* Generate sample private key */
    eth_private_key_t private_key;
    generate_sample_private_key(&private_key);
    
    /* Create a sample EIP-1559 transaction */
    eth_transaction_t tx;
    create_sample_eip1559_transaction(&tx);
    
    printf("Unsigned EIP-1559 transaction:\n");
    printf("- Chain ID: %lu\n", tx.chain_id);
    printf("- Nonce: %lu\n", tx.nonce);
    printf("- Max priority fee: 0x");
    print_hex(tx.max_priority_fee, tx.max_priority_fee_len);
    printf("\n");
    printf("- Max fee: 0x");
    print_hex(tx.max_fee, tx.max_fee_len);
    printf("\n");
    printf("- Gas limit: %lu\n", tx.gas_limit);
    printf("- To: 0x");
    print_hex(tx.to, tx.to_len);
    printf("\n");
    printf("- Value: 0x");
    print_hex(tx.value, tx.value_len);
    printf("\n");
    
    /* Create buffer for encoded transaction */
    uint8_t encoded_tx[1024];
    size_t encoded_size;
    
    /* Encode the unsigned transaction */
    int result = eth_tx_encode(&tx, encoded_tx, sizeof(encoded_tx), &encoded_size);
    
    printf("Encoded unsigned transaction: 0x");
    print_hex(encoded_tx, encoded_size);
    printf("\n");
    
    /* Hash the transaction for signing */
    eth_hash_t tx_hash;
    result = eth_tx_hash(&tx, &tx_hash);
    
    printf("Transaction hash: 0x");
    print_hex(tx_hash.data, 32);
    printf("\n");
    
    /* Sign the transaction */
    result = eth_tx_sign(&tx, &private_key);
    
    printf("Signature V: %u\n", tx.v);
    printf("Signature R: 0x");
    print_hex(tx.r, 32);
    printf("\n");
    printf("Signature S: 0x");
    print_hex(tx.s, 32);
    printf("\n");
    
    /* Encode the signed transaction */
    result = eth_tx_encode_signed(&tx, encoded_tx, sizeof(encoded_tx), &encoded_size);
    
    printf("Encoded signed transaction: 0x");
    print_hex(encoded_tx, encoded_size);
    printf("\n");
}

/* Demo function to showcase RLP encoding */
static void demo_rlp_encoding(void) {
    printf("\n--- RLP Encoding Demo ---\n");
    
    /* Create a buffer for RLP encoding */
    uint8_t buffer[256];
    rlp_encoder_t encoder;
    
    /* Initialize the encoder */
    rlp_encoder_init(&encoder, buffer, sizeof(buffer));
    
    /* Encode a single byte */
    printf("Encoding a single byte (0x42)...\n");
    rlp_encode_byte(&encoder, 0x42);
    
    printf("Result: 0x");
    print_hex(buffer, rlp_get_length(&encoder));
    printf("\n");
    
    /* Reset encoder */
    rlp_encoder_init(&encoder, buffer, sizeof(buffer));
    
    /* Encode a short string */
    printf("Encoding a short string ('dog')...\n");
    rlp_encode_bytes(&encoder, (const uint8_t *)"dog", 3);
    
    printf("Result: 0x");
    print_hex(buffer, rlp_get_length(&encoder));
    printf("\n");
    
    /* Reset encoder */
    rlp_encoder_init(&encoder, buffer, sizeof(buffer));
    
    /* Encode a list of two strings */
    printf("Encoding a list of two strings ('cat', 'dog')...\n");
    
    size_t list_marker;
    rlp_begin_list(&encoder, &list_marker);
    rlp_encode_bytes(&encoder, (const uint8_t *)"cat", 3);
    rlp_encode_bytes(&encoder, (const uint8_t *)"dog", 3);
    rlp_end_list(&encoder, list_marker);
    
    printf("Result: 0x");
    print_hex(buffer, rlp_get_length(&encoder));
    printf("\n");
    
    /* Reset encoder */
    rlp_encoder_init(&encoder, buffer, sizeof(buffer));
    
    /* Encode a nested list */
    printf("Encoding a nested list...\n");
    
    size_t outer_list_marker;
    rlp_begin_list(&encoder, &outer_list_marker);
    
    rlp_encode_bytes(&encoder, (const uint8_t *)"cat", 3);
    
    size_t inner_list_marker;
    rlp_begin_list(&encoder, &inner_list_marker);
    rlp_encode_bytes(&encoder, (const uint8_t *)"dog", 3);
    rlp_encode_bytes(&encoder, (const uint8_t *)"pig", 3);
    rlp_end_list(&encoder, inner_list_marker);
    
    rlp_encode_bytes(&encoder, (const uint8_t *)"owl", 3);
    
    rlp_end_list(&encoder, outer_list_marker);
    
    printf("Result: 0x");
    print_hex(buffer, rlp_get_length(&encoder));
    printf("\n");
}

/* Demo function to showcase contract interaction */
static void demo_contract_interaction(void) {
    printf("\n--- Contract Interaction Demo ---\n");
    
    /* Generate sample private key */
    eth_private_key_t private_key;
    generate_sample_private_key(&private_key);
    
    /* Create a sample contract interaction transaction */
    eth_transaction_t tx;
    create_sample_contract_interaction(&tx);
    
    printf("Contract interaction transaction:\n");
    printf("- To (contract): 0x");
    print_hex(tx.to, tx.to_len);
    printf("\n");
    printf("- Data (function call): 0x");
    print_hex(tx.data, tx.data_len);
    printf("\n");
    
    /* Sign the transaction */
    int result = eth_tx_sign(&tx, &private_key);
    
    /* Encode the signed transaction */
    uint8_t encoded_tx[1024];
    size_t encoded_size;
    result = eth_tx_encode_signed(&tx, encoded_tx, sizeof(encoded_tx), &encoded_size);
    
    printf("Encoded signed contract interaction: 0x");
    print_hex(encoded_tx, encoded_size);
    printf("\n");
}

int main(void) {
    printf("Ethereum Embedded Signer Demo\n");
    printf("=============================\n");
    
    /* Run demos */
    demo_crypto_operations();
    demo_rlp_encoding();
    demo_transaction_signing();
    demo_contract_interaction();
    
    printf("\nDemo completed successfully!\n");
    
    return 0;
} 