# Kalyna Block Cipher - C Reference Implementation

Reference implementation of the **Kalyna block cipher** (DSTU 7624:2014), the Ukrainian national encryption standard.

## Overview

Kalyna is a symmetric block cipher supporting multiple block and key sizes:
- **Block sizes**: 128, 256, 512 bits
- **Key sizes**: 128, 256, 512 bits
- **Rounds**: 10 (128-bit key), 14 (256-bit key), 18 (512-bit key)

## Project Structure

```
.
├── kalyna.h              # Main API header
├── kalyna.c              # Core cipher implementation
├── transformations.h     # Internal transformations
├── tables.h              # S-boxes and MDS matrices header
├── tables.c              # S-boxes and MDS matrices data
├── main.c                # Test vectors validation
├── benchmark.c           # Performance benchmarking
├── makefile              # Build configuration
└── README.md             # This file
```

## Building

### Library and Tests
```bash
make                    # Build and run tests
make kalyna-reference   # Build reference implementation
```

### Benchmark
```bash
gcc kalyna.c benchmark.c tables.c -o kalyna-benchmark -O3
./kalyna-benchmark
```

**Optimization flags for best performance:**
```bash
gcc kalyna.c benchmark.c tables.c -o kalyna-benchmark -O3 -march=native -DNDEBUG
```

## API Reference

### Data Types

#### `kalyna_t`
Cipher context containing state and configuration:
```c
typedef struct {
    size_t nb;              // Number of 64-bit words in block
    size_t nk;              // Number of 64-bit words in key
    size_t nr;              // Number of rounds
    uint64_t* state;        // Current cipher state
    uint64_t** round_keys;  // Precomputed round keys
} kalyna_t;
```

### Functions

#### `kalyna_t* KalynaInit(size_t block_size, size_t key_size)`
Initialize cipher context.

**Parameters:**
- `block_size`: Block size in bits (128, 256, or 512)
- `key_size`: Key size in bits (must be equal or double the block size)

**Returns:** Pointer to initialized context, or NULL on error

**Supported combinations:**
- Kalyna-128/128: `KalynaInit(128, 128)`
- Kalyna-128/256: `KalynaInit(128, 256)`
- Kalyna-256/256: `KalynaInit(256, 256)`
- Kalyna-256/512: `KalynaInit(256, 512)`
- Kalyna-512/512: `KalynaInit(512, 512)`

**Example:**
```c
kalyna_t* ctx = KalynaInit(128, 128);
if (!ctx) {
    fprintf(stderr, "Initialization failed\n");
    return -1;
}
```

---

#### `void KalynaKeyExpand(uint64_t* key, kalyna_t* ctx)`
Generate and store round keys from master key.

**Parameters:**
- `key`: Master encryption key as array of 64-bit words
- `ctx`: Initialized cipher context

**Note:** Must be called before encryption/decryption

**Example:**
```c
uint64_t key[2] = {0x0706050403020100ULL, 0x0f0e0d0c0b0a0908ULL};
KalynaKeyExpand(key, ctx);
```

---

#### `void KalynaEncipher(uint64_t* plaintext, kalyna_t* ctx, uint64_t* ciphertext)`
Encrypt a single block.

**Parameters:**
- `plaintext`: Input block as array of 64-bit words
- `ctx`: Initialized context with expanded keys
- `ciphertext`: Output buffer for encrypted block

**Example:**
```c
uint64_t pt[2] = {0x1716151413121110ULL, 0x1f1e1d1c1b1a1918ULL};
uint64_t ct[2];
KalynaEncipher(pt, ctx, ct);
```

---

#### `void KalynaDecipher(uint64_t* ciphertext, kalyna_t* ctx, uint64_t* plaintext)`
Decrypt a single block.

**Parameters:**
- `ciphertext`: Encrypted block as array of 64-bit words
- `ctx`: Initialized context with expanded keys
- `plaintext`: Output buffer for decrypted block

**Example:**
```c
uint64_t pt[2];
KalynaDecipher(ct, ctx, pt);
```

---

#### `int KalynaDelete(kalyna_t* ctx)`
Free cipher context and allocated memory.

**Parameters:**
- `ctx`: Context to delete

**Returns:** 0 on success

**Example:**
```c
KalynaDelete(ctx);
```

## Usage Example

### Complete Encryption/Decryption

```c
#include <stdio.h>
#include <string.h>
#include "kalyna.h"

int main() {
    // Initialize Kalyna-128/128
    kalyna_t* ctx = KalynaInit(128, 128);
    if (!ctx) return -1;

    // Prepare key (128 bits = 2 x 64-bit words)
    uint64_t key[2] = {
        0x0706050403020100ULL,
        0x0f0e0d0c0b0a0908ULL
    };

    // Prepare plaintext
    uint64_t plaintext[2] = {
        0x1716151413121110ULL,
        0x1f1e1d1c1b1a1918ULL
    };

    uint64_t ciphertext[2];
    uint64_t decrypted[2];

    // Expand key
    KalynaKeyExpand(key, ctx);

    // Encrypt
    KalynaEncipher(plaintext, ctx, ciphertext);
    printf("Ciphertext: %016llx %016llx\n", 
           ciphertext[0], ciphertext[1]);

    // Decrypt
    KalynaDecipher(ciphertext, ctx, decrypted);

    // Verify
    if (memcmp(plaintext, decrypted, sizeof(plaintext)) == 0) {
        printf("Success: Decryption matches plaintext\n");
    } else {
        printf("Error: Decryption mismatch\n");
    }

    // Cleanup
    KalynaDelete(ctx);
    return 0;
}
```

### Working with Different Block Sizes

#### Kalyna-256/256
```c
kalyna_t* ctx = KalynaInit(256, 256);
uint64_t key[4];      // 256 bits = 4 words
uint64_t plaintext[4]; // 256 bits = 4 words
uint64_t ciphertext[4];
```

#### Kalyna-512/512
```c
kalyna_t* ctx = KalynaInit(512, 512);
uint64_t key[8];      // 512 bits = 8 words
uint64_t plaintext[8]; // 512 bits = 8 words
uint64_t ciphertext[8];
```

## Data Format

### Endianness
- The implementation handles both little-endian and big-endian architectures automatically
- Data is internally processed in little-endian format
- Byte ordering is handled transparently by the library

### Word Size
- All blocks and keys are represented as arrays of **64-bit unsigned integers** (`uint64_t`)
- For 128-bit block: 2 words
- For 256-bit block: 4 words
- For 512-bit block: 8 words

### Byte Order in Words
Words follow little-endian convention:
```
Word: 0x0706050403020100
Bytes: 00 01 02 03 04 05 06 07 (in memory)
```

## Test Vectors

The `main.c` file contains official test vectors from DSTU 7624:2014 standard. Run tests:

```bash
make
```

Expected output:
```
=============
Kalyna (128, 128)
--- ENCIPHERING ---
Success enciphering

--- DECIPHERING ---
Success deciphering
...
```

## Performance

Run benchmarks to measure performance on your system:

```bash
gcc kalyna.c benchmark.c tables.c -o kalyna-benchmark -O3 -march=native
./kalyna-benchmark
```

Example output:
```
=== Kalyna-128/128 ===
Block size: 128 bits, Key size: 128 bits

Key Expansion:  0.023 ms

Encryption:
  Total time:   245.3 ms
  Time/op:      2.45 µs
  Throughput:   408,163 ops/sec
  Throughput:   6,530 MB/s

Decryption:
  Total time:   251.7 ms
  Time/op:      2.52 µs
  Throughput:   397,308 ops/sec
  Throughput:   6,357 MB/s
```

## Implementation Notes

### Security Considerations
- This is a **reference implementation** focused on clarity, not performance
- For production use, consider:
  - Constant-time implementations to prevent timing attacks
  - Hardware acceleration (AES-NI for similar operations)
  - Side-channel attack mitigations

### Memory Management
- Caller is responsible for allocating input/output buffers
- `KalynaInit()` allocates memory for context - must call `KalynaDelete()`
- No internal buffering - processes one block at a time

### Thread Safety
- Context (`kalyna_t`) is **not thread-safe**
- Each thread should use its own context
- Key expansion can be done once and shared (read-only after expansion)

## Comparison with Rust Implementation

To compare with Rust version on the same machine:

### C Benchmark
```bash
gcc kalyna.c benchmark.c tables.c -o kalyna-benchmark -O3 -march=native
./kalyna-benchmark
```

### Rust Benchmark
```bash
cd rust-kalyna
cargo bench --release
```

### Metrics to Compare
- **Throughput** (MB/s or ops/sec)
- **Latency** (µs per operation)
- **Key expansion time**
- **Binary size**
- **Memory usage**

## Authors

Original C implementation:
- Ruslan Kiianchuk
- Ruslan Mordvinov  
- Roman Oliynykov

## Standards Compliance

This implementation conforms to:
- **DSTU 7624:2014** - Ukrainian National Standard for Block Cipher

## License

Reference implementation for educational and standardization purposes.

## References

- [DSTU 7624:2014 Standard](https://www.dsszzi.gov.ua/dsszzi/control/uk/publish/article?art_id=46113)
- Kalyna cipher specification and test vectors