#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "kalyna.h"

#define BENCHMARK_ITERATIONS 100000
#define WARMUP_ITERATIONS 1000

typedef struct {
    const char* name;
    size_t block_size;
    size_t key_size;
} BenchmarkConfig;

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

void benchmark_variant(BenchmarkConfig config) {
    kalyna_t* ctx = KalynaInit(config.block_size, config.key_size);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize Kalyna context\n");
        return;
    }

    // Prepare test data
    size_t block_words = config.block_size / 64;
    size_t key_words = config.key_size / 64;
    
    uint64_t* key = (uint64_t*)calloc(key_words, sizeof(uint64_t));
    uint64_t* plaintext = (uint64_t*)calloc(block_words, sizeof(uint64_t));
    uint64_t* ciphertext = (uint64_t*)calloc(block_words, sizeof(uint64_t));
    uint64_t* decrypted = (uint64_t*)calloc(block_words, sizeof(uint64_t));

    // Initialize with test pattern
    for (size_t i = 0; i < key_words; i++) {
        key[i] = 0x0706050403020100ULL + (i * 0x0808080808080808ULL);
    }
    for (size_t i = 0; i < block_words; i++) {
        plaintext[i] = 0x1716151413121110ULL + (i * 0x0808080808080808ULL);
    }

    // Key expansion (done once)
    double key_exp_start = get_time_ms();
    KalynaKeyExpand(key, ctx);
    double key_exp_time = get_time_ms() - key_exp_start;

    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        KalynaEncipher(plaintext, ctx, ciphertext);
        KalynaDecipher(ciphertext, ctx, decrypted);
    }

    // Benchmark encryption
    double enc_start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        KalynaEncipher(plaintext, ctx, ciphertext);
    }
    double enc_time = get_time_ms() - enc_start;

    // Benchmark decryption
    double dec_start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        KalynaDecipher(ciphertext, ctx, decrypted);
    }
    double dec_time = get_time_ms() - dec_start;

    // Verify correctness
    if (memcmp(plaintext, decrypted, block_words * sizeof(uint64_t)) != 0) {
        fprintf(stderr, "ERROR: Decryption mismatch!\n");
    }

    // Calculate statistics
    double enc_ops_per_sec = (BENCHMARK_ITERATIONS * 1000.0) / enc_time;
    double dec_ops_per_sec = (BENCHMARK_ITERATIONS * 1000.0) / dec_time;
    double enc_mb_per_sec = (enc_ops_per_sec * config.block_size) / (8.0 * 1024 * 1024);
    double dec_mb_per_sec = (dec_ops_per_sec * config.block_size) / (8.0 * 1024 * 1024);

    // Print results
    printf("\n=== %s ===\n", config.name);
    printf("Block size: %zu bits, Key size: %zu bits\n", config.block_size, config.key_size);
    printf("Iterations: %d\n\n", BENCHMARK_ITERATIONS);
    
    printf("Key Expansion:  %.3f ms\n\n", key_exp_time);
    
    printf("Encryption:\n");
    printf("  Total time:   %.3f ms\n", enc_time);
    printf("  Time/op:      %.3f µs\n", (enc_time * 1000) / BENCHMARK_ITERATIONS);
    printf("  Throughput:   %.2f ops/sec\n", enc_ops_per_sec);
    printf("  Throughput:   %.2f MB/s\n\n", enc_mb_per_sec);
    
    printf("Decryption:\n");
    printf("  Total time:   %.3f ms\n", dec_time);
    printf("  Time/op:      %.3f µs\n", (dec_time * 1000) / BENCHMARK_ITERATIONS);
    printf("  Throughput:   %.2f ops/sec\n", dec_ops_per_sec);
    printf("  Throughput:   %.2f MB/s\n", dec_mb_per_sec);

    // Cleanup
    free(key);
    free(plaintext);
    free(ciphertext);
    free(decrypted);
    KalynaDelete(ctx);
}

void print_system_info() {
    printf("=== Kalyna Block Cipher Benchmark ===\n");
    printf("Compiled with: %s\n", __VERSION__);
    printf("Date: %s %s\n", __DATE__, __TIME__);
    
    #ifdef __x86_64__
    printf("Architecture: x86_64\n");
    #elif __aarch64__
    printf("Architecture: ARM64\n");
    #else
    printf("Architecture: Unknown\n");
    #endif
    
    printf("\nBenchmark parameters:\n");
    printf("  Iterations: %d\n", BENCHMARK_ITERATIONS);
    printf("  Warmup iterations: %d\n", WARMUP_ITERATIONS);
}

int main(int argc, char** argv) {
    print_system_info();

    BenchmarkConfig configs[] = {
        {"Kalyna-128/128", 128, 128},
        {"Kalyna-128/256", 128, 256},
        {"Kalyna-256/256", 256, 256},
        {"Kalyna-256/512", 256, 512},
        {"Kalyna-512/512", 512, 512}
    };

    for (size_t i = 0; i < sizeof(configs) / sizeof(configs[0]); i++) {
        benchmark_variant(configs[i]);
    }

    printf("\n=== Benchmark Complete ===\n");
    return 0;
}
