#include "FOS_Rng.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int main(void)
{
    FOS_Rng rng = FOS_rng_default();

    uint32_t num;

    for (size_t i = 0; i < 9; ++i)
    {
        num = FOS_rng_u32(&rng);

        printf("%u ", num);
    }
    putchar('\n');

    printf("%u\n", FOS_rng_range_gen(&rng, 0, 10));
    printf("%u\n", FOS_rng_range_gen(&rng, 0, 10));
    printf("%u\n", FOS_rng_range_gen(&rng, 0, 0));
    printf("%u\n", FOS_rng_range_gen(&rng, 0, UINT32_MAX));
    printf("%u\n", FOS_rng_range_gen(&rng, UINT32_MAX, UINT32_MAX));
    printf("%u\n", FOS_rng_range(0, 10));
    printf("%u\n", FOS_rng_range(0, 10));
    printf("%u\n", FOS_rng_range(0, 10));

    uint64_t val64 = FOS_rng_u64(&rng);

    printf("64-bit val: %llu\n", val64);

    printf("%llu\n", FOS_rng_range_gen_u64(&rng, 0, 10));
    printf("%llu\n", FOS_rng_range_gen_u64(&rng, 0, 10));
    printf("%llu\n", FOS_rng_range_gen_u64(&rng, 0, 0));
    printf("%llu\n", FOS_rng_range_gen_u64(&rng, 0, 57849322));
    printf("%llu\n", FOS_rng_range_gen_u64(&rng, UINT64_MAX, UINT64_MAX));

    printf("%f\n", FOS_rng_f64(&rng));
    printf("%f\n", FOS_rng_f64(&rng));

    int arr[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

    assert(FOS_rng_shuffle_int(arr, 9, &rng));

    for (size_t i = 0; i < 9; ++i)
        printf("%d ", arr[i]);

    putchar('\n');
/**************************************************************/

    FOS_Rng rng2 = FOS_rng_seed(396783259ULL, 943562367704ULL);

    uint32_t num2;

    for (size_t i = 0; i < 9; ++i)
    {
        num2 = FOS_rng_u32(&rng2);

        printf("%u ", num2);
    }
    putchar('\n');

    uint64_t new_val64 = FOS_rng_u64(&rng2);

    printf("64-bit val: %llu\n", new_val64);

    putchar('\n');
/**************************************************************/

    FOS_Rng rng3 = FOS_rng_seed_auto();

    uint32_t num3;

    for (size_t i = 0; i < 9; ++i)
    {
        num3 = FOS_rng_u32(&rng3);

        printf("%u ", num3);
    }
    putchar('\n');

    uint64_t another_val64 = FOS_rng_u64(&rng3);

    printf("64-bit val: %llu\n", another_val64);

    uint64_t arr_ll[5] = { 84638509213ULL, 53859207551299ULL, 9756734821457609ULL, 
                                           8379430921357653ULL, 7851354753098135789ULL}; 

    assert(FOS_rng_shuffle_generic(arr_ll, 5, sizeof(uint64_t), &rng3));

    for (size_t i = 0; i < 5; ++i)
        printf("%llu ", arr_ll[i]);
    putchar('\n');

    uint64_t res = *(uint64_t *)FOS_rng_choose(arr_ll, 5, sizeof(uint64_t), &rng3);

    printf("Selected value: %llu\n", res);
    
    printf("All tests passed.\n");  
}