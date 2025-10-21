/*

Optimized implementation of the Kalyna block cipher (DSTU 7624:2014)

Key optimizations:
- Eliminated all dynamic allocations in hot paths
- In-place operations using stack buffers
- Fixed all compiler warnings
- Optimized type conversions
- Better cache locality

Authors: Ruslan Kiianchuk, Ruslan Mordvinov, Roman Oliynykov
Optimized by: Performance improvements for production use

*/

#include "transformations.h"
#include "tables.h"

// Maximum buffer size for 512-bit blocks
#define MAX_STATE_BYTES 64


kalyna_t* KalynaInit(size_t block_size, size_t key_size) {
    kalyna_t* ctx = (kalyna_t*)malloc(sizeof(kalyna_t));
    if (!ctx) return NULL;

    if (block_size == kBLOCK_128) {
        ctx->nb = kBLOCK_128 / kBITS_IN_WORD;
        if (key_size == kKEY_128) {
            ctx->nk = kKEY_128 / kBITS_IN_WORD;
            ctx->nr = kNR_128;
        } else if (key_size == kKEY_256){
            ctx->nk =  kKEY_256 / kBITS_IN_WORD;
            ctx->nr = kNR_256;
        } else {
            fprintf(stderr, "Error: unsupported key size.\n");
            free(ctx);
            return NULL;
        }
    } else if (block_size == 256) {
        ctx->nb = kBLOCK_256 / kBITS_IN_WORD;
        if (key_size == kKEY_256) {
            ctx->nk = kKEY_256 / kBITS_IN_WORD;
            ctx->nr = kNR_256;
        } else if (key_size == kKEY_512){
            ctx->nk = kKEY_512 / kBITS_IN_WORD;
            ctx->nr = kNR_512;
        } else {
            fprintf(stderr, "Error: unsupported key size.\n");
            free(ctx);
            return NULL;
        }
    } else if (block_size == kBLOCK_512) {
        ctx->nb = kBLOCK_512 / kBITS_IN_WORD;
        if (key_size == kKEY_512) {
            ctx->nk = kKEY_512 / kBITS_IN_WORD;
            ctx->nr = kNR_512;
        } else {
            fprintf(stderr, "Error: unsupported key size.\n");
            free(ctx);
            return NULL;
        }
    } else {
        fprintf(stderr, "Error: unsupported block size.\n");
        free(ctx);
        return NULL;
    }

    ctx->state = (uint64_t*)calloc(ctx->nb, sizeof(uint64_t));
    if (ctx->state == NULL) {
        perror("Could not allocate memory for cipher state.");
        free(ctx);
        return NULL;
    }

    ctx->round_keys = (uint64_t**)calloc(ctx->nr + 1, sizeof(uint64_t*));
    if (ctx->round_keys == NULL) {
        perror("Could not allocate memory for cipher round keys.");
        free(ctx->state);
        free(ctx);
        return NULL;
    }

    for (size_t i = 0; i < ctx->nr + 1; ++i) {
        ctx->round_keys[i] = (uint64_t*)calloc(ctx->nb, sizeof(uint64_t));
        if (ctx->round_keys[i] == NULL) {
            perror("Could not allocate memory for cipher round keys.");
            for (size_t j = 0; j < i; ++j) {
                free(ctx->round_keys[j]);
            }
            free(ctx->round_keys);
            free(ctx->state);
            free(ctx);
            return NULL;
        }
    }
    return ctx;
}


int KalynaDelete(kalyna_t* ctx) {
    if (!ctx) return 0;
    
    if (ctx->state) free(ctx->state);
    
    if (ctx->round_keys) {
        for (size_t i = 0; i < ctx->nr + 1; ++i) {
            if (ctx->round_keys[i]) free(ctx->round_keys[i]);
        }
        free(ctx->round_keys);
    }
    
    free(ctx);
    return 0;
}


void SubBytes(kalyna_t* ctx) {
    uint64_t* s = ctx->state;
    for (size_t i = 0; i < ctx->nb; ++i) {
        ctx->state[i] = sboxes_enc[0][s[i] & 0x00000000000000FFULL] |
            ((uint64_t)sboxes_enc[1][(s[i] & 0x000000000000FF00ULL) >> 8] << 8) |
            ((uint64_t)sboxes_enc[2][(s[i] & 0x0000000000FF0000ULL) >> 16] << 16) |
            ((uint64_t)sboxes_enc[3][(s[i] & 0x00000000FF000000ULL) >> 24] << 24) |
            ((uint64_t)sboxes_enc[0][(s[i] & 0x000000FF00000000ULL) >> 32] << 32) |
            ((uint64_t)sboxes_enc[1][(s[i] & 0x0000FF0000000000ULL) >> 40] << 40) |
            ((uint64_t)sboxes_enc[2][(s[i] & 0x00FF000000000000ULL) >> 48] << 48) |
            ((uint64_t)sboxes_enc[3][(s[i] & 0xFF00000000000000ULL) >> 56] << 56);
    }
}

void InvSubBytes(kalyna_t* ctx) {
    uint64_t* s = ctx->state;
    for (size_t i = 0; i < ctx->nb; ++i) {
        ctx->state[i] = sboxes_dec[0][s[i] & 0x00000000000000FFULL] |
            ((uint64_t)sboxes_dec[1][(s[i] & 0x000000000000FF00ULL) >> 8] << 8) |
            ((uint64_t)sboxes_dec[2][(s[i] & 0x0000000000FF0000ULL) >> 16] << 16) |
            ((uint64_t)sboxes_dec[3][(s[i] & 0x00000000FF000000ULL) >> 24] << 24) |
            ((uint64_t)sboxes_dec[0][(s[i] & 0x000000FF00000000ULL) >> 32] << 32) |
            ((uint64_t)sboxes_dec[1][(s[i] & 0x0000FF0000000000ULL) >> 40] << 40) |
            ((uint64_t)sboxes_dec[2][(s[i] & 0x00FF000000000000ULL) >> 48] << 48) |
            ((uint64_t)sboxes_dec[3][(s[i] & 0xFF00000000000000ULL) >> 56] << 56);
    }
}


// OPTIMIZED: No dynamic allocation, uses stack buffers
static inline void WordsToBytes_Inline(size_t nb, const uint64_t* words, uint8_t* bytes) {
    if (IsBigEndian()) {
        for (size_t i = 0; i < nb; ++i) {
            uint64_t w = ReverseWord(words[i]);
            memcpy(bytes + i * 8, &w, 8);
        }
    } else {
        memcpy(bytes, words, nb * sizeof(uint64_t));
    }
}

static inline void BytesToWords_Inline(size_t nb, const uint8_t* bytes, uint64_t* words) {
    memcpy(words, bytes, nb * sizeof(uint64_t));
    if (IsBigEndian()) {
        for (size_t i = 0; i < nb; ++i) {
            words[i] = ReverseWord(words[i]);
        }
    }
}


void ShiftRows(kalyna_t* ctx) {
    // OPTIMIZED: Stack allocation instead of malloc
    uint8_t state[MAX_STATE_BYTES];
    uint8_t nstate[MAX_STATE_BYTES];
    
    WordsToBytes_Inline(ctx->nb, ctx->state, state);
    
    size_t shift = 0;
    for (size_t row = 0; row < 8; ++row) {
        if (row % (8 / ctx->nb) == 0 && row > 0)
            shift += 1;
        for (size_t col = 0; col < ctx->nb; ++col) {
            nstate[row + (col + shift) % ctx->nb * 8] = state[row + col * 8];
        }
    }

    BytesToWords_Inline(ctx->nb, nstate, ctx->state);
}

void InvShiftRows(kalyna_t* ctx) {
    // OPTIMIZED: Stack allocation instead of malloc
    uint8_t state[MAX_STATE_BYTES];
    uint8_t nstate[MAX_STATE_BYTES];
    
    WordsToBytes_Inline(ctx->nb, ctx->state, state);
    
    size_t shift = 0;
    for (size_t row = 0; row < 8; ++row) {
        if (row % (8 / ctx->nb) == 0 && row > 0)
            shift += 1;
        for (size_t col = 0; col < ctx->nb; ++col) {
            nstate[row + col * 8] = state[row + (col + shift) % ctx->nb * 8];
        }
    }

    BytesToWords_Inline(ctx->nb, nstate, ctx->state);
}


uint8_t MultiplyGF(uint8_t x, uint8_t y) {
    uint8_t r = 0;
    uint8_t hbit;
    for (size_t i = 0; i < 8; ++i) {
        if ((y & 0x1) == 1)
            r ^= x;
        hbit = x & 0x80;
        x <<= 1;
        if (hbit == 0x80)
            x ^= kREDUCTION_POLYNOMIAL;
        y >>= 1;
    }
    return r;
}

void MatrixMultiply(kalyna_t* ctx, uint8_t matrix[8][8]) {
    // OPTIMIZED: Stack buffer, single conversion
    uint8_t state[MAX_STATE_BYTES];
    WordsToBytes_Inline(ctx->nb, ctx->state, state);

    for (size_t col = 0; col < ctx->nb; ++col) {
        uint64_t result = 0;
        for (int row = 7; row >= 0; --row) {
            uint8_t product = 0;
            for (int b = 7; b >= 0; --b) {
                product ^= MultiplyGF(state[b + col * 8], matrix[row][b]);
            }
            result |= (uint64_t)product << (row * 8);
        }    
        ctx->state[col] = result;
    }
}

void MixColumns(kalyna_t* ctx) {
    MatrixMultiply(ctx, mds_matrix);
}

void InvMixColumns(kalyna_t* ctx) {
    MatrixMultiply(ctx, mds_inv_matrix);
}


void EncipherRound(kalyna_t* ctx) {
    SubBytes(ctx);
    ShiftRows(ctx);
    MixColumns(ctx);
}

void DecipherRound(kalyna_t* ctx) {
    InvMixColumns(ctx);
    InvShiftRows(ctx);
    InvSubBytes(ctx);
}

void AddRoundKey(int round, kalyna_t* ctx) {
    for (size_t i = 0; i < ctx->nb; ++i) {
        ctx->state[i] = ctx->state[i] + ctx->round_keys[round][i];
    }
}

void SubRoundKey(int round, kalyna_t* ctx) {
    for (size_t i = 0; i < ctx->nb; ++i) {
        ctx->state[i] = ctx->state[i] - ctx->round_keys[round][i];
    }
}

void AddRoundKeyExpand(uint64_t* value, kalyna_t* ctx) {
    for (size_t i = 0; i < ctx->nb; ++i) {
        ctx->state[i] = ctx->state[i] + value[i];
    }
}

void XorRoundKey(int round, kalyna_t* ctx) {
    for (size_t i = 0; i < ctx->nb; ++i) {
        ctx->state[i] = ctx->state[i] ^ ctx->round_keys[round][i];
    }
}

void XorRoundKeyExpand(uint64_t* value, kalyna_t* ctx) {
    for (size_t i = 0; i < ctx->nb; ++i) {
        ctx->state[i] = ctx->state[i] ^ value[i];
    }
}

void Rotate(size_t state_size, uint64_t* state_value) {
    uint64_t temp = state_value[0];
    for (size_t i = 1; i < state_size; ++i) {
        state_value[i - 1] = state_value[i];
    }
    state_value[state_size - 1] = temp;
}

void ShiftLeft(size_t state_size, uint64_t* state_value) {
    for (size_t i = 0; i < state_size; ++i) {
        state_value[i] <<= 1;
    } 
}

void RotateLeft(size_t state_size, uint64_t* state_value) {
    size_t rotate_bytes = 2 * state_size + 3;
    size_t bytes_num = state_size * 8;

    // OPTIMIZED: Stack buffer instead of malloc
    uint8_t bytes[MAX_STATE_BYTES];
    uint8_t buffer[MAX_STATE_BYTES];
    
    WordsToBytes_Inline(state_size, state_value, bytes);

    memcpy(buffer, bytes, rotate_bytes);
    memmove(bytes, bytes + rotate_bytes, bytes_num - rotate_bytes);
    memcpy(bytes + bytes_num - rotate_bytes, buffer, rotate_bytes);

    BytesToWords_Inline(state_size, bytes, state_value);
}


void KeyExpandKt(uint64_t* key, kalyna_t* ctx, uint64_t* kt) {
    // OPTIMIZED: Stack allocation
    uint64_t k0[8];
    uint64_t k1[8];
  
  memset(ctx->state, 0, ctx->nb * sizeof(uint64_t));
    ctx->state[0] += ctx->nb + ctx->nk + 1;
     
    if (ctx->nb == ctx->nk) {
        memcpy(k0, key, ctx->nb * sizeof(uint64_t));
        memcpy(k1, key, ctx->nb * sizeof(uint64_t));
    } else {
        memcpy(k0, key, ctx->nb * sizeof(uint64_t));
        memcpy(k1, key + ctx->nb, ctx->nb * sizeof(uint64_t));
    }

    AddRoundKeyExpand(k0, ctx);
    EncipherRound(ctx);
    XorRoundKeyExpand(k1, ctx);
    EncipherRound(ctx);
    AddRoundKeyExpand(k0, ctx);
    EncipherRound(ctx);
    memcpy(kt, ctx->state, ctx->nb * sizeof(uint64_t));
}


void KeyExpandEven(uint64_t* key, uint64_t* kt, kalyna_t* ctx) {
    // OPTIMIZED: Stack allocation
    uint64_t initial_data[8];
    uint64_t kt_round[8];
    uint64_t tmv[8];
  size_t round = 0;

    memcpy(initial_data, key, ctx->nk * sizeof(uint64_t));
    for (size_t i = 0; i < ctx->nb; ++i) {
        tmv[i] = 0x0001000100010001ULL;
    }

    while(1) {
        memcpy(ctx->state, kt, ctx->nb * sizeof(uint64_t));
        AddRoundKeyExpand(tmv, ctx);
        memcpy(kt_round, ctx->state, ctx->nb * sizeof(uint64_t));

        memcpy(ctx->state, initial_data, ctx->nb * sizeof(uint64_t));

        AddRoundKeyExpand(kt_round, ctx);
        EncipherRound(ctx);
        XorRoundKeyExpand(kt_round, ctx);
        EncipherRound(ctx);
        AddRoundKeyExpand(kt_round, ctx);

        memcpy(ctx->round_keys[round], ctx->state, ctx->nb * sizeof(uint64_t));

        if (ctx->nr == round)
            break;

        if (ctx->nk != ctx->nb) {
            round += 2;

            ShiftLeft(ctx->nb, tmv);

            memcpy(ctx->state, kt, ctx->nb * sizeof(uint64_t));
            AddRoundKeyExpand(tmv, ctx);
            memcpy(kt_round, ctx->state, ctx->nb * sizeof(uint64_t));

            memcpy(ctx->state, initial_data + ctx->nb, ctx->nb * sizeof(uint64_t));

            AddRoundKeyExpand(kt_round, ctx);
            EncipherRound(ctx);
            XorRoundKeyExpand(kt_round, ctx);
            EncipherRound(ctx);
            AddRoundKeyExpand(kt_round, ctx);

            memcpy(ctx->round_keys[round], ctx->state, ctx->nb * sizeof(uint64_t));

            if (ctx->nr == round)
                break;
        }
        round += 2;
        ShiftLeft(ctx->nb, tmv);
        Rotate(ctx->nk, initial_data);
    }
}

void KeyExpandOdd(kalyna_t* ctx) {
    for (size_t i = 1; i < ctx->nr; i += 2) {
        memcpy(ctx->round_keys[i], ctx->round_keys[i - 1], ctx->nb * sizeof(uint64_t));
        RotateLeft(ctx->nb, ctx->round_keys[i]);
    }
}

void KalynaKeyExpand(uint64_t* key, kalyna_t* ctx) {
    // OPTIMIZED: Stack allocation
    uint64_t kt[8];
    KeyExpandKt(key, ctx, kt);
    KeyExpandEven(key, kt, ctx);
    KeyExpandOdd(ctx);
}


void KalynaEncipher(uint64_t* plaintext, kalyna_t* ctx, uint64_t* ciphertext) {
    memcpy(ctx->state, plaintext, ctx->nb * sizeof(uint64_t));

    AddRoundKey(0, ctx);
    for (size_t round = 1; round < ctx->nr; ++round) {
        EncipherRound(ctx);
        XorRoundKey(round, ctx);
    }
    EncipherRound(ctx);
    AddRoundKey(ctx->nr, ctx);

    memcpy(ciphertext, ctx->state, ctx->nb * sizeof(uint64_t));
}

void KalynaDecipher(uint64_t* ciphertext, kalyna_t* ctx, uint64_t* plaintext) {
    memcpy(ctx->state, ciphertext, ctx->nb * sizeof(uint64_t));

    SubRoundKey(ctx->nr, ctx);
    for (size_t round = ctx->nr - 1; round > 0; --round) {
        DecipherRound(ctx);
        XorRoundKey(round, ctx);
    }
    DecipherRound(ctx);
    SubRoundKey(0, ctx);

    memcpy(plaintext, ctx->state, ctx->nb * sizeof(uint64_t));
}


// Keep these for compatibility with original API
uint8_t* WordsToBytes(size_t length, uint64_t* words) {
    uint8_t* bytes = (uint8_t*)words;
    if (IsBigEndian()) {
        for (size_t i = 0; i < length; ++i) {
            words[i] = ReverseWord(words[i]);
        }        
    }
    return bytes;
}

uint64_t* BytesToWords(size_t length, uint8_t* bytes) {
    uint64_t* words = (uint64_t*)bytes;
    if (IsBigEndian()) {
        for (size_t i = 0; i < length / 8; ++i) {
            words[i] = ReverseWord(words[i]);
        }        
    }
    return words;
}


uint64_t ReverseWord(uint64_t word) {
    uint64_t reversed = 0;
    uint8_t* src = (uint8_t*)&word;
    uint8_t* dst = (uint8_t*)&reversed;

    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        dst[i] = src[sizeof(uint64_t) - 1 - i];    
    }
    return reversed;
}


int IsBigEndian(void) {
    unsigned int num = 1;
    return (*((uint8_t*)&num) == 0);
}

void PrintState(size_t length, uint64_t* state) {
    for (size_t i = length; i > 0; --i) {
        printf("%16.16llx", state[i - 1]);
    } 
    printf("\n");
}