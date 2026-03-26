#ifndef FOS_BIGNUM_H
#define FOS_BIGNUM_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#if defined(_MSC_VER)
    #define FOS_THREAD_LOCAL __declspec(thread)
#else
    #define FOS_THREAD_LOCAL _Thread_local
#endif

#define FOS_BIGNUM_BASE 1000000000u

FOS_THREAD_LOCAL static size_t fos_bignum_max_digits = 0; // Unbounded

typedef struct {
    uint32_t *digits;
    size_t size;
    size_t capacity;
    int sign;
} FOS_Bignum;

bool FOS_bignum_init(FOS_Bignum *bn);
void FOS_bignum_set_max_digits(size_t max);
void FOS_bignum_free(FOS_Bignum *bn);
bool FOS_bignum_reserve(FOS_Bignum *bn, size_t cap);
bool FOS_bignum_from_u64(FOS_Bignum *bn, uint64_t value);
bool FOS_bignum_trim(FOS_Bignum *bn);
bool FOS_bignum_copy(FOS_Bignum *dst, const FOS_Bignum *src);
int FOS_bignum_cmp(const FOS_Bignum *fst, const FOS_Bignum *snd);
int FOS_bignum_cmp_abs(const FOS_Bignum *fst, const FOS_Bignum *snd);
bool FOS_bignum_add_u32(FOS_Bignum *bn, uint32_t d);
bool FOS_bignum_sub_u32(FOS_Bignum *bn, uint32_t d);
bool FOS_bignum_mul_u32(FOS_Bignum *bn, uint32_t d);
bool FOS_bignum_div_u32(FOS_Bignum *bn, uint32_t d, uint32_t *rem_out);
bool FOS_bignum_add(FOS_Bignum *res, const FOS_Bignum *a, const FOS_Bignum *b);
bool FOS_bignum_subtract(FOS_Bignum *res, const FOS_Bignum *a, const FOS_Bignum *b);
bool FOS_bignum_multiply(FOS_Bignum *res, const FOS_Bignum *a, const FOS_Bignum *b);
bool FOS_bignum_divide(FOS_Bignum *quot, FOS_Bignum *rem, const FOS_Bignum *a, const FOS_Bignum *b);
bool FOS_bignum_from_cstr(FOS_Bignum *bn, const char *num);
bool FOS_bignum_to_cstr(const FOS_Bignum *bn, char *num, size_t buf);

#endif
