/*
 * Benchmark for Kalyna block cipher
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "kalyna.h"

#define ITERATIONS 100000
#define WARMUP 1000

static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

static void benchmark_variant(size_t block_size, size_t key_size, const char* name) {
    size_t nb = block_size / 64;
    size_t nk = key_size / 64;

    uint64_t* key = (uint64_t*)calloc(nk, sizeof(uint64_t));
    uint64_t* plaintext = (uint64_t*)calloc(nb, sizeof(uint64_t));
    uint64_t* ciphertext = (uint64_t*)calloc(nb, sizeof(uint64_t));

    /* Initialize with test data */
    for (size_t i = 0; i < nk; i++) key[i] = 0x0001020304050607ULL + i;
    for (size_t i = 0; i < nb; i++) plaintext[i] = 0x1011121314151617ULL + i;

    kalyna_t* ctx = KalynaInit(block_size, key_size);
    if (!ctx) {
        fprintf(stderr, "Failed to init %s\n", name);
        free(key); free(plaintext); free(ciphertext);
        return;
    }

    /* Key expansion benchmark */
    double start = get_time_ms();
    for (int i = 0; i < WARMUP; i++) {
        KalynaKeyExpand(key, ctx);
    }
    double key_expand_time = (get_time_ms() - start) / WARMUP;

    /* Warmup */
    KalynaKeyExpand(key, ctx);
    for (int i = 0; i < WARMUP; i++) {
        KalynaEncipher(plaintext, ctx, ciphertext);
    }

    /* Encryption benchmark */
    start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        KalynaEncipher(plaintext, ctx, ciphertext);
    }
    double enc_time = get_time_ms() - start;

    /* Decryption benchmark */
    start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        KalynaDecipher(ciphertext, ctx, plaintext);
    }
    double dec_time = get_time_ms() - start;

    /* Results */
    double enc_throughput = (ITERATIONS * block_size / 8.0) / (enc_time / 1000.0) / (1024 * 1024);
    double dec_throughput = (ITERATIONS * block_size / 8.0) / (dec_time / 1000.0) / (1024 * 1024);

    printf("=== %s ===\n", name);
    printf("Key expansion:  %.3f us/op\n", key_expand_time * 1000);
    printf("Encryption:     %.3f us/op  (%.2f MB/s)\n",
           enc_time * 1000 / ITERATIONS, enc_throughput);
    printf("Decryption:     %.3f us/op  (%.2f MB/s)\n",
           dec_time * 1000 / ITERATIONS, dec_throughput);
    printf("\n");

    KalynaDelete(ctx);
    free(key);
    free(plaintext);
    free(ciphertext);
}

int main(void) {
    printf("Kalyna Benchmark (%d iterations)\n", ITERATIONS);
    printf("================================\n\n");

    benchmark_variant(128, 128, "Kalyna-128/128");
    benchmark_variant(128, 256, "Kalyna-128/256");
    benchmark_variant(256, 256, "Kalyna-256/256");
    benchmark_variant(256, 512, "Kalyna-256/512");
    benchmark_variant(512, 512, "Kalyna-512/512");

    return 0;
}
