/*
 * Constant and basic transformations for the reference implementation of the
 * Kalyna block cipher (DSTU 7624:2014)
 *
 * Authors: Ruslan Kiianchuk, Ruslan Mordvinov, Roman Oliynykov
 *
 * Note: All transformation functions are now declared static in kalyna.c
 * and are not part of the public API. This header only contains constants
 * and macros used internally.
 */

#ifndef KALYNA_DEFS_H
#define KALYNA_DEFS_H


#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <limits.h>

#include "kalyna.h"


#if (ULLONG_MAX != 0xFFFFFFFFFFFFFFFFULL)
#error "Architecture not supported. Required type to fit 64 bits."
#endif

#define kBITS_IN_WORD 64

#if (CHAR_BIT != 8)
#error "Architecture not supported. Required type to fit 8 bits."
#endif

#define kBITS_IN_BYTE 8

#define TRUE 1
#define FALSE 0

/* Block words size. */
#define kNB_128 2
#define kNB_256 4
#define kNB_512 8

/* Key words size. */
#define kNK_128 2
#define kNK_256 4
#define kNK_512 8

/* Block bits size. */
#define kBLOCK_128 (kNB_128 * kBITS_IN_WORD)
#define kBLOCK_256 (kNB_256 * kBITS_IN_WORD)
#define kBLOCK_512 (kNB_512 * kBITS_IN_WORD)

/* Key bits size. */
#define kKEY_128 (kNK_128 * kBITS_IN_WORD)
#define kKEY_256 (kNK_256 * kBITS_IN_WORD)
#define kKEY_512 (kNK_512 * kBITS_IN_WORD)

/* Number of enciphering rounds size depending on key length. */
#define kNR_128 10
#define kNR_256 14
#define kNR_512 18

#define kREDUCTION_POLYNOMIAL 0x011d  /* x^8 + x^4 + x^3 + x^2 + 1 */

/*!
 * Index a byte array as cipher state matrix.
 */
#define INDEX(table, row, col) table[(row) + (col) * sizeof(uint64_t)]

#endif  /* KALYNA_DEFS_H */

