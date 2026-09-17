// Compilation from MSYS2 UCRT64 shell:
// gcc fos_hashmap_ex.c -I../include -L.. -lfos -o fos_hashmap_ex

#include "FOS_HashMap.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    FOS_HashMap hmap = FOS_hashmap_new(sizeof(int), sizeof(char *), NULL, NULL);

    if (!FOS_hashmap_is_valid(&hmap))
    {
        fprintf(stderr, "Map init failed.\n");
        exit(EXIT_FAILURE);
    }

    const char *value_9 = "nine", 
               *value_5 = "five", 
               *value_3 = "three";

    // The map stores copies of the pointer values, not the strings.
    FOS_hashmap_put(&hmap, &(int){9}, &value_9);
    FOS_hashmap_put(&hmap, &(int){5}, &value_5);
    FOS_hashmap_put(&hmap, &(int){3}, &value_3);

    const char *out;

    if (FOS_hashmap_get(&hmap, &(int){5}, &out))
        printf("%s\n", out);

    FOS_hashmap_free(&hmap);

    return 0;
}