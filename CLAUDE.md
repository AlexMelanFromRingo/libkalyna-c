# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Reference implementation of the Kalyna block cipher (DSTU 7624:2014), the Ukrainian national encryption standard. Supports all block and key length variants.

## Build Commands

All commands run from the `src/` directory:

```bash
make          # Build the kalyna executable
make test     # Build and run test vectors
make clean    # Remove compiled objects and executable
```

## Architecture

### Core Components

- **kalyna.h** - Public API: `KalynaInit`, `KalynaDelete`, `KalynaKeyExpand`, `KalynaEncipher`, `KalynaDecipher`
- **kalyna.c** - Main cipher implementation with key expansion and encryption/decryption routines
- **transformations.h** - Internal cipher transformations (SubBytes, ShiftRows, MixColumns, key operations)
- **tables.c/tables.h** - S-boxes (`sboxes_enc`, `sboxes_dec`) and MDS matrices

### Key Data Structure

```c
typedef struct {
    size_t nb;              // Number of 64-bit words in block
    size_t nk;              // Number of 64-bit words in key
    size_t nr;              // Number of encryption rounds
    uint64_t* state;        // Cipher state
    uint64_t** round_keys;  // Precomputed round keys
} kalyna_t;
```

### Supported Variants

| Block Size | Key Size | Rounds |
|------------|----------|--------|
| 128 bits   | 128 bits | 10     |
| 128 bits   | 256 bits | 14     |
| 256 bits   | 256 bits | 14     |
| 256 bits   | 512 bits | 18     |
| 512 bits   | 512 bits | 18     |

### Platform Requirements

- 64-bit `uint64_t` support
- 8-bit chars (`CHAR_BIT == 8`)
- Supports both little-endian and big-endian architectures
