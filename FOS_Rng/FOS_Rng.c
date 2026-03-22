#include "FOS_Rng.h"

FOS_Rng FOS_rng_default(void)
{
    return (FOS_Rng) { .state = 359ULL, .inc = 953ULL };
}

FOS_Rng FOS_rng_seed(uint64_t seed, uint64_t stream)
{
    FOS_Rng rng;

    rng.state = 0;
    rng.inc = (stream << 1u) | 1u;

    FOS_rng_u32(&rng);
    rng.state += seed;
    FOS_rng_u32(&rng);

    return rng;
}

FOS_Rng FOS_rng_seed_auto(void)
{
    static uint64_t counter = 0;
    uint64_t var = 0;
    uint64_t *ptr = NULL;

    uint64_t seed1 = counter ^ (uint64_t)(time(NULL) ^ (uintptr_t)&var);
    uint64_t seed2 = counter ^ ((uint64_t)(uintptr_t)&ptr >> 8);

    counter += 35995335901ULL;

    return FOS_rng_seed(seed1, seed2);
}

uint32_t FOS_rng_u32(FOS_Rng *rng)
{
    uint64_t old_state = rng->state;

    rng->state = old_state * 6364136223846793005ULL + rng->inc;

    uint32_t xor_shifted = (uint32_t)(((old_state >> 18u) ^ old_state) >> 27u);
    uint32_t rot = old_state >> 59u;

    return (xor_shifted >> rot) | (xor_shifted << ((32 - rot) & 31));
}

uint32_t FOS_rng_range_gen(FOS_Rng *rng, uint32_t min, uint32_t max)
{
    if (max == UINT32_MAX && min == 0)
        return FOS_rng_u32(rng);

    if (min > max)
    {
        uint32_t temp = min;
        min = max;
        max = temp;
    }

    uint32_t range = max - min + 1;

    if (range == 1)
        return min;

    if (range == 2)
        return min + (FOS_rng_u32(rng) & 1);

    // uint32_t tail = ((-range) % range);
    uint32_t tail = (UINT32_MAX - range + 1) % range;
    uint32_t num;

    do {
        num = FOS_rng_u32(rng);
    } while (num < tail);

    uint32_t result = min + (num % range);

    return result;
}

uint32_t FOS_rng_range(uint32_t min, uint32_t max)
{
    static FOS_Rng default_rng;
    static bool init = false;

    if (!init) 
    { 
        default_rng = FOS_rng_seed_auto();
        init = true; 
    }

    return FOS_rng_range_gen(&default_rng, min, max); 
}

uint64_t FOS_rng_u64(FOS_Rng *rng)
{
    return ((uint64_t)FOS_rng_u32(rng) << 32) | FOS_rng_u32(rng);
}

double FOS_rng_f64(FOS_Rng *rng)
{
    uint64_t fst = ((uint64_t)FOS_rng_u32(rng) << 32) | FOS_rng_u32(rng);

    uint64_t snd = fst >> 11;
    double res = (double)snd / 9007199254740992.0; // 2^53

    return res;
}
    
bool FOS_rng_shuffle_int(int *arr, size_t n, FOS_Rng *rng)
{
    if (arr == NULL || n == 0 || rng == NULL)
        return false;

    for (size_t i = n - 1; i > 0; --i)
    {
        size_t j = FOS_rng_range_gen(rng, 0, (uint32_t)i);

        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }

    return true;
}

bool FOS_rng_shuffle_generic(void *data, size_t n, size_t elem_size, FOS_Rng *rng)
{
    if (data == NULL || n == 0 || elem_size == 0 || rng == NULL)
        return false;

    char *temp = malloc(elem_size);

    if (temp == NULL)
        return false;

    for (size_t i = n - 1; i > 0; --i)
    {
        size_t j = FOS_rng_range_gen(rng, 0, (uint32_t)i);

        memmove(temp, (char *)data + i * elem_size, elem_size);
        memmove((char *)data + i * elem_size, (char *)data + j * elem_size, elem_size);
        memmove((char *)data + j * elem_size, temp, elem_size);
    }
    
    free(temp);

    return true;
}

void *FOS_rng_choose(void *data, size_t n, size_t elem_size, FOS_Rng *rng)
{
    if (!data || n == 0 || elem_size == 0 || !rng)
        return NULL;

    size_t i = FOS_rng_range_gen(rng, 0, (uint32_t)(n - 1));

    return (char *)data + i * elem_size;
}
