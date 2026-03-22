#ifndef FOS_RNG_H
#define FOS_RNG_H

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint64_t state;
    uint64_t inc;
} FOS_Rng;

FOS_Rng FOS_rng_default(void);
FOS_Rng FOS_rng_seed(uint64_t seed, uint64_t stream);
FOS_Rng FOS_rng_seed_auto(void);
uint32_t FOS_rng_u32(FOS_Rng *rng);
uint32_t FOS_rng_range_gen(FOS_Rng *rng, uint32_t min, uint32_t max);
uint32_t FOS_rng_range(uint32_t min, uint32_t max);
uint64_t FOS_rng_u64(FOS_Rng *rng);
double FOS_rng_f64(FOS_Rng *rng);
bool FOS_rng_shuffle_int(int *arr, size_t n, FOS_Rng *rng);
bool FOS_rng_shuffle_generic(void *data, size_t n, size_t elem_size, FOS_Rng *rng);
void *FOS_rng_choose(void *data, size_t n, size_t elem_size, FOS_Rng *rng);

#endif
