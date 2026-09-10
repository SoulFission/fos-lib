#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "FOS_Bignum.h"

int main(void)
{
/*******************************************************/
    FOS_Bignum bnum = { 0 };

    bool initialized = FOS_bignum_init(&bnum);

    assert(initialized);

    assert(FOS_bignum_from_u64(&bnum, 359018421095371296ULL));

    FOS_Bignum bnum2 = { 0 };

    initialized = FOS_bignum_init(&bnum2);

    assert(initialized);

    assert(FOS_bignum_from_cstr(&bnum2, "359018421095371296"));

    assert(FOS_bignum_cmp(&bnum, &bnum2) == 0);

    FOS_bignum_free(&bnum);
    FOS_bignum_free(&bnum2);

/*******************************************************/

    FOS_Bignum bnum3 = { 0 };

    assert(FOS_bignum_init(&bnum3));

    assert(FOS_bignum_from_cstr(&bnum3, "00000359018421095371296"));
    assert(FOS_bignum_trim(&bnum3));

    char num[64];

    assert(FOS_bignum_to_cstr(&bnum3, num, 64));
    assert(strcmp(num, "359018421095371296") == 0);

    FOS_bignum_free(&bnum3);

/*******************************************************/

    FOS_Bignum bnum4 = { 0 }, bnum5 = { 0 };

    assert(FOS_bignum_init(&bnum4));
    assert(FOS_bignum_init(&bnum5));

    assert(FOS_bignum_from_cstr(&bnum4, "953104349527098143756935673189325"));
    assert(FOS_bignum_copy(&bnum5, &bnum4));

    assert(FOS_bignum_cmp(&bnum5, &bnum4) == 0);

    FOS_bignum_free(&bnum4);
    FOS_bignum_free(&bnum5);

/*******************************************************/

    FOS_Bignum bnum6 = { 0 };

    assert(FOS_bignum_init(&bnum6));
    assert(FOS_bignum_from_cstr(&bnum6, "359"));

    assert(FOS_bignum_add_u32(&bnum6, 953u));

    char num2[64];

    assert(FOS_bignum_to_cstr(&bnum6, num2, 64));

    assert(strcmp(num2, "1312") == 0);

    FOS_bignum_free(&bnum6);

/******************************************************/

    FOS_Bignum bnum7 = { 0 };

    assert(FOS_bignum_init(&bnum7));
    assert(FOS_bignum_from_cstr(&bnum7, "100000000000000000000000000000000009"));

    assert(FOS_bignum_sub_u32(&bnum7, 10));

    char num3[64];

    assert(FOS_bignum_to_cstr(&bnum7, num3, 64));

    assert(strcmp(num3, "99999999999999999999999999999999999") == 0);

    FOS_bignum_free(&bnum7);

/*******************************************************/

    FOS_Bignum bnum8 = { 0 };

    assert(FOS_bignum_init(&bnum8));
    assert(FOS_bignum_from_cstr(&bnum8, "359"));

    assert(FOS_bignum_mul_u32(&bnum8, 953));

    char num4[64];

    assert(FOS_bignum_to_cstr(&bnum8, num4, 64));

    assert(strcmp(num4, "342127") == 0);

    FOS_bignum_free(&bnum8);

/********************************************************/

    FOS_Bignum bnum9 = { 0 };

    uint32_t rem = 0;

    assert(FOS_bignum_init(&bnum9));
    assert(FOS_bignum_from_cstr(&bnum9, "359953"));

    assert(FOS_bignum_div_u32(&bnum9, 359, &rem));

    char num5[64];

    assert(FOS_bignum_to_cstr(&bnum9, num5, 64));

    assert(strcmp(num5, "1002") == 0);

    assert(rem == 235u);

    FOS_bignum_free(&bnum9);

/*******************************************************/

    FOS_Bignum bnum10 = { 0 }, bnum11 = { 0 };

    assert(FOS_bignum_init(&bnum10));
    assert(FOS_bignum_init(&bnum11));

    assert(FOS_bignum_from_cstr(&bnum10, "8974352790766543312807563498"));
    assert(FOS_bignum_from_cstr(&bnum11, "5983670913578976498342908"));

    // FOS_bignum_add(&bnum10, &bnum10, &bnum11);
    // FOS_bignum_add(&bnum10, &bnum11, &bnum10);
    FOS_bignum_add(&bnum10, &bnum10, &bnum10);

    char num6[64];

    assert(FOS_bignum_to_cstr(&bnum10, num6, 64));

    // assert(strcmp(num6, "8980336461680122289305906406") == 0);
    assert(strcmp(num6, "17948705581533086625615126996") == 0);

    FOS_bignum_free(&bnum10);
    FOS_bignum_free(&bnum11);

/*******************************************************/

    FOS_Bignum bnum12 = { 0 }, bnum13 = { 0 };

    assert(FOS_bignum_init(&bnum12));
    assert(FOS_bignum_init(&bnum13));

    assert(FOS_bignum_from_cstr(&bnum12, "8974352790766543312807563498"));
    assert(FOS_bignum_from_cstr(&bnum13, "5983670913578976498342908"));

    // FOS_bignum_subtract(&bnum12, &bnum12, &bnum13);
    FOS_bignum_subtract(&bnum12, &bnum12, &bnum12);

    char num7[64];

    assert(FOS_bignum_to_cstr(&bnum12, num7, 64));

    // assert(strcmp(num7, "8968369119852964336309220590") == 0);
    assert(strcmp(num7, "0") == 0);

    FOS_bignum_free(&bnum12);
    FOS_bignum_free(&bnum13);

/********************************************************/

    FOS_Bignum bnum14 = { 0 }, bnum15 = { 0 };

    assert(FOS_bignum_init(&bnum14));
    assert(FOS_bignum_init(&bnum15)); 

    assert(FOS_bignum_from_cstr(&bnum14, "89743527907"));
    assert(FOS_bignum_from_cstr(&bnum15, "5983670913578976"));   

    //FOS_bignum_multiply(&bnum14, &bnum14, &bnum15);
    FOS_bignum_multiply(&bnum14, &bnum14, &bnum14);

    char num8[64];

    assert(FOS_bignum_to_cstr(&bnum14, num8, 64));

    assert(strcmp(num8, "8053900801194487800649") == 0);
    //assert(strcmp(num8, "536995737619079017904483232") == 0);

    FOS_bignum_free(&bnum14);
    FOS_bignum_free(&bnum15);

/*******************************************************/

    FOS_Bignum bnum16 = { 0 }, bnum17 = { 0 }, brem = { 0 };

    assert(FOS_bignum_init(&bnum16));
    assert(FOS_bignum_init(&bnum17));
    assert(FOS_bignum_init(&brem));

    assert(FOS_bignum_from_cstr(&bnum16, "476926509135632877643217"));
    assert(FOS_bignum_from_cstr(&bnum17, "598367091"));

    FOS_bignum_divide(&bnum16, &brem, &bnum16, &bnum17);

    char num9[64], num10[64];

    assert(FOS_bignum_to_cstr(&bnum16, num9, 64));
    assert(FOS_bignum_to_cstr(&brem, num10, 64));

    assert(strcmp(num9, "797046689747904") == 0);
    assert(strcmp(num10, "37815953") == 0);

    FOS_bignum_free(&bnum16);
    FOS_bignum_free(&bnum17);
    FOS_bignum_free(&brem);

    printf("All tests passed.\n");

    return 0;
}
