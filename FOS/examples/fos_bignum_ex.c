// Compilation from MSYS2 UCRT64 shell:
// gcc fos_bignum_ex.c -I../include -L.. -lfos -o fos_bignum_ex

#include "FOS.h"
#include <stdio.h>

#define BUF_SIZE 64

int main(void)
{
    FOS_Bignum a;
    FOS_Bignum b;
    FOS_Bignum result;

    FOS_bignum_init(&a);
    FOS_bignum_init(&b);
    FOS_bignum_init(&result);

    FOS_bignum_from_cstr(&a, "12345678901234567890");
    FOS_bignum_from_cstr(&b, "98765432109876543210");

    FOS_bignum_add(&result, &a, &b);

    char num[BUF_SIZE];

    FOS_bignum_to_cstr(&result, num, BUF_SIZE);

    printf("Result is %s\n", num); // 111111111011111111100

    FOS_bignum_free(&a);
    FOS_bignum_free(&b);
    FOS_bignum_free(&result);

    return 0;
}
