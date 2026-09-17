// Compilation from MSYS2 UCRT64 shell:
// gcc fos_rng_ex.c -I../include -L.. -lfos -o fos_rng_ex

#include "FOS_Rng.h"
#include <stdio.h>

int main(void)
{
    FOS_Rng rng = FOS_rng_seed(2359, 9532);

    printf("Random uint32: %u\n", FOS_rng_u32(&rng));
    printf("Random uint64: %llu\n",
           (unsigned long long)FOS_rng_u64(&rng));
    printf("Random double: %f\n", FOS_rng_f64(&rng));

    printf("Random values from 1 to 10:\n");

    for (int i = 0; i < 5; ++i)
        printf("%u ", FOS_rng_range_gen(&rng, 1, 10));

    putchar('\n');

    int values[] = { 1, 2, 3, 4, 5, 6 };

    if (FOS_rng_shuffle_int(values,
                            sizeof(values) / sizeof(values[0]),
                            &rng))
    {
        printf("Shuffled array: ");

        for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
            printf("%d ", values[i]);

        putchar('\n');
    }

    int *chosen = FOS_rng_choose(values,
                                 sizeof(values) / sizeof(values[0]),
                                 sizeof(values[0]),
                                 &rng);

    if (chosen != NULL)
        printf("Randomly chosen element: %d\n", *chosen);

    return 0;
}
