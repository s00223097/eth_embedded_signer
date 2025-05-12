# Ethereum Embedded Signer

An extremely basic implementation of Ethereum transaction signing - related to my chainstaterealty project. 

Thanks to the inspiration and ideas provided by the code in the following projects on Github:
- https://github.com/paulmillr/micro-eth-signer
- https://github.com/pubkey/eth-crypto

## Features

> Most of these are at least somewhat implemented but they are not polished or ready for anything other than haphazard dev testing right now. I haven't done much with C up till now, the code probably reflects that and it's also why the comments are very obvious and almost every fx has 3-4 lines explaining every part. 

- **Ethereum Transaction Support**:
  - Legacy transactions (pre-EIP-2718)
  - EIP-2930 transactions (with access list)
  - EIP-1559 transactions (priority fee; changeable)

- **Cryptographic Operations**:
  - Keccak-256 hashing (barebones implementation)
  - ECDSA signing and verification on the secp256k1 curve (barebones implementation)
  - Public key to Ethereum address derivation

- **RLP Encoding**:
  - Full implementation of Recursive Length Prefix encoding used by Ethereum
  - Support for single values, byte arrays, and nested lists
  - Memory-efficient encoding with a single-pass algorithm that avoids double buffering
  - Streaming RLP encoding that processes data incrementally
  - Constant memory usage regardless of input size

- **Contract Interaction**:
  - Support for smart contract function calls
  - ABI encoding (attempted effort)

## Building

### Prerequisites

- CMake 3.10 or newer
- C compiler with C11 (Only C11...)

```bash
mkdir -p build && cd build
cmake ..
make
```

## Usage

There's a single file demo "app" in `src/main.c` that showcases:

1. Basic cryptographic operations
2. RLP encoding examples
3. Ethereum transaction creation and signing
4. Smart contract interaction

Running the demo after building: just go into it

```bash
./eth_signer
```

## Integration (theoretical)

This is how I would go about integrating this into the main project on paper:

1. Include the necessary header files:
   ```c
   #include "crypto.h"
   #include "rlp.h"
   #include "transaction.h"
   ```

2. Create and sign a transaction:
   ```c
   // 1. Start a transaction
   eth_transaction_t tx;
   eth_tx_init(&tx, ETH_EIP1559_TX);
   
   // 2. Fill in Transaction parameters
   tx.chain_id = 1;  // Ethereum mainnet - idk what value shoudl be here exactly e.g. if you use a testnet it will probably be a node index or address?
   tx.nonce = 42;
   // ... any other parameters
   
   // 3. Sign transaction
   eth_private_key_t private_key;
   // load the private key wherever you have it
   eth_tx_sign(&tx, &private_key);
   
   // 4. Encode signed transaction
   uint8_t buffer[1024];
   size_t encoded_size;
   eth_tx_encode_signed(&tx, buffer, sizeof(buffer), &encoded_size);
   
   ```

Resulting transaction is in buffer with length encoded_size or should be... (still being tested)
