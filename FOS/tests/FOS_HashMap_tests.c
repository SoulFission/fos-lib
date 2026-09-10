#include <stdio.h>
#include <assert.h>
#include "FOS_HashMap.h"

int main(void)
{
/****************************************************************************************/
    FOS_HashMap hm = FOS_hashmap_new(sizeof(int), sizeof(char *), FOS_fnv1a, FOS_key_eq);

    assert(FOS_hashmap_is_valid(&hm));

    const char *three = "three";

    assert(FOS_hashmap_put(&hm, &(int) { 3 }, &three));

    assert(FOS_hashmap_contains(&hm, &(int) { 3 }));

    char *result = NULL;

    assert(FOS_hashmap_get(&hm, &(int) { 3 }, &result));

    puts(three);

    assert(FOS_hashmap_remove(&hm, &(int) { 3 }));

    assert(!FOS_hashmap_contains(&hm, &(int) { 3 }));

    FOS_hashmap_free(&hm);

/****************************************************************************************/

    FOS_HashMap hm2 = FOS_hashmap_new(sizeof(int), sizeof(long long), FOS_fnv1a, FOS_key_eq);

    assert(FOS_hashmap_is_valid(&hm2));

    int i = 0;
    long long j = 2;

    for (; i < 150; ++i, j += 2)
        assert(FOS_hashmap_put(&hm2, &i, &j));

    long long vals[150] = { 0 };

    for (int k = 0; k < 150; ++k)
        assert(FOS_hashmap_get(&hm2, &k, &vals[k]));

    for (int z = 0; z < 150; ++z)
        printf("%lld\n", vals[z]);

    FOS_hashmap_free(&hm2);

    puts("All tests passed.");

    return 0;
}
