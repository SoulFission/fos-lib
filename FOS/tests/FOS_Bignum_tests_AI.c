/*
 * FOS_Bignum test harness
 *
 * The tests deliberately concentrate on:
 *
 *   - construction/conversion
 *   - comparison
 *   - copying
 *   - normalization / leading zeros
 *   - add/subtract/multiply/divide
 *   - carries and borrows across many digits
 *   - zero and one
 *   - quotient/remainder
 *   - in-place / aliased operations
 *   - output-buffer boundaries
 *
 * Recommended:
 *
 *   GCC / Clang:
 *       -fsanitize=address,undefined
 *
 *       -Wall -Wextra -Wpedantic
 *
 * This harness assumes the FOS_Bignum API used by the current module:
 *
 *   FOS_bignum_init
 *   FOS_bignum_free
 *   FOS_bignum_from_u64
 *   FOS_bignum_from_cstr
 *   FOS_bignum_to_cstr
 *   FOS_bignum_trim
 *   FOS_bignum_cmp
 *   FOS_bignum_copy
 *   FOS_bignum_add_u32
 *   FOS_bignum_sub_u32
 *   FOS_bignum_mul_u32
 *   FOS_bignum_div_u32
 *   FOS_bignum_add
 *   FOS_bignum_subtract
 *   FOS_bignum_multiply
 *   FOS_bignum_divide
 */

#include "FOS_Bignum.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ------------------------------------------------------------------------- */
/* Test helpers                                                              */
/* ------------------------------------------------------------------------- */

#define CHECK(expr)                                                         \
    do                                                                      \
    {                                                                       \
        if (!(expr))                                                        \
        {                                                                   \
            fprintf(stderr,                                                  \
                    "FAIL: %s\n    at %s:%d\n",                             \
                    #expr, __FILE__, __LINE__);                             \
            exit(EXIT_FAILURE);                                             \
        }                                                                    \
    } while (0)


static void expect_string(FOS_Bignum *bnum, const char *expected)
{
    /*
     * 1024 is intentionally generous for the deterministic values used
     * below. The test is about arithmetic, not exercising the exact
     * output-buffer limit here.
     */
    char actual[1024];

    CHECK(FOS_bignum_to_cstr(bnum, actual, sizeof actual));
    CHECK(strcmp(actual, expected) == 0);
}


static void expect_cmp(FOS_Bignum *a, FOS_Bignum *b, int expected)
{
    CHECK(FOS_bignum_cmp(a, b) == expected);
}


static void expect_from_cstr(FOS_Bignum *bnum, const char *value)
{
    CHECK(FOS_bignum_from_cstr(bnum, value));
}


static void init_bignum(FOS_Bignum *bnum)
{
    *bnum = (FOS_Bignum){ 0 };
    CHECK(FOS_bignum_init(bnum));
}


/* ------------------------------------------------------------------------- */
/* Construction / conversion                                                */
/* ------------------------------------------------------------------------- */

static void test_construction(void)
{
    puts("  construction/conversion");

    FOS_Bignum a;
    FOS_Bignum b;

    init_bignum(&a);
    init_bignum(&b);

    CHECK(FOS_bignum_from_u64(&a, 359018421095371296ULL));
    CHECK(FOS_bignum_from_cstr(&b, "359018421095371296"));

    expect_string(&a, "359018421095371296");
    expect_string(&b, "359018421095371296");

    expect_cmp(&a, &b, 0);

    FOS_bignum_free(&a);
    FOS_bignum_free(&b);
}


/* ------------------------------------------------------------------------- */
/* Leading zeros / trim                                                      */
/* ------------------------------------------------------------------------- */

static void test_leading_zeros(void)
{
    puts("  leading zeros/trim");

    FOS_Bignum bnum;
    init_bignum(&bnum);

    expect_from_cstr(&bnum, "000000000000000000359");

    CHECK(FOS_bignum_trim(&bnum));
    expect_string(&bnum, "359");

    /*
     * Zero with excessive leading zeros.
     */
    CHECK(FOS_bignum_from_cstr(&bnum, "000000000000000000000"));

    CHECK(FOS_bignum_trim(&bnum));
    expect_string(&bnum, "0");

    FOS_bignum_free(&bnum);
}


/* ------------------------------------------------------------------------- */
/* Copy                                                                      */
/* ------------------------------------------------------------------------- */

static void test_copy(void)
{
    puts("  copy");

    FOS_Bignum src;
    FOS_Bignum dst;

    init_bignum(&src);
    init_bignum(&dst);

    expect_from_cstr(
        &src,
        "953104349527098143756935673189325"
    );

    CHECK(FOS_bignum_copy(&dst, &src));

    expect_cmp(&src, &dst, 0);

    /*
     * Make sure this really is a copy rather than shared state.
     */
    CHECK(FOS_bignum_add_u32(&src, 1));
    expect_string(&src, "953104349527098143756935673189326");
    expect_string(&dst, "953104349527098143756935673189325");

    FOS_bignum_free(&src);
    FOS_bignum_free(&dst);
}


/* ------------------------------------------------------------------------- */
/* Comparison                                                                */
/* ------------------------------------------------------------------------- */

static void test_comparison(void)
{
    puts("  comparison");

    FOS_Bignum a;
    FOS_Bignum b;

    init_bignum(&a);
    init_bignum(&b);

    expect_from_cstr(&a, "123456789");
    expect_from_cstr(&b, "123456789");

    expect_cmp(&a, &b, 0);

    expect_from_cstr(&b, "123456790");
    expect_cmp(&a, &b, -1);

    expect_cmp(&b, &a, 1);

    /*
     * Same numerical value with different textual representation.
     */
    expect_from_cstr(&a, "000000123456789");
    expect_cmp(&a, &b, -1);

    expect_from_cstr(&b, "123456789");
    expect_cmp(&a, &b, 0);

    /*
     * Different number of digits.
     */
    expect_from_cstr(&a, "999999999999");
    expect_from_cstr(&b, "1000000000000");

    expect_cmp(&a, &b, -1);
    expect_cmp(&b, &a, 1);

    FOS_bignum_free(&a);
    FOS_bignum_free(&b);
}


/* ------------------------------------------------------------------------- */
/* Small integer arithmetic                                                   */
/* ------------------------------------------------------------------------- */

static void test_u32_arithmetic(void)
{
    puts("  u32 arithmetic");

    FOS_Bignum bnum;
    init_bignum(&bnum);

    /*
     * Addition.
     */
    expect_from_cstr(&bnum, "359");
    CHECK(FOS_bignum_add_u32(&bnum, 953u));
    expect_string(&bnum, "1312");

    /*
     * Subtraction with borrow.
     */
    expect_from_cstr(
        &bnum,
        "100000000000000000000000000000000009"
    );

    CHECK(FOS_bignum_sub_u32(&bnum, 10));
    expect_string(
        &bnum,
        "99999999999999999999999999999999999"
    );

    /*
     * Multiplication.
     */
    expect_from_cstr(&bnum, "359");
    CHECK(FOS_bignum_mul_u32(&bnum, 953u));
    expect_string(&bnum, "342127");

    /*
     * Division with remainder.
     */
    uint32_t rem = 0;

    expect_from_cstr(&bnum, "359953");

    CHECK(FOS_bignum_div_u32(&bnum, 359u, &rem));

    expect_string(&bnum, "1002");
    CHECK(rem == 235u);

    FOS_bignum_free(&bnum);
}


/* ------------------------------------------------------------------------- */
/* Zero and one                                                               */
/* ------------------------------------------------------------------------- */

static void test_zero_one(void)
{
    puts("  zero/one identities");

    FOS_Bignum a;
    FOS_Bignum b;
    FOS_Bignum result;
    FOS_Bignum rem;

    init_bignum(&a);
    init_bignum(&b);
    init_bignum(&result);
    init_bignum(&rem);

    expect_from_cstr(&a, "0");
    expect_from_cstr(
        &b,
        "123456789012345678901234567890"
    );

    /*
     * 0 + x
     */
    CHECK(FOS_bignum_add(&result, &a, &b));
    expect_string(&result, "123456789012345678901234567890");

    /*
     * x + 0
     */
    CHECK(FOS_bignum_add(&result, &b, &a));
    expect_string(&result, "123456789012345678901234567890");

    /*
     * x - 0
     */
    CHECK(FOS_bignum_subtract(&result, &b, &a));
    expect_string(&result, "123456789012345678901234567890");

    /*
     * x * 0
     */
    CHECK(FOS_bignum_multiply(&result, &b, &a));
    expect_string(&result, "0");

    /*
     * 0 * x
     */
    CHECK(FOS_bignum_multiply(&result, &a, &b));
    expect_string(&result, "0");

    /*
     * 0 / x
     */
    CHECK(FOS_bignum_divide(&result, &rem, &a, &b));
    expect_string(&result, "0");
    expect_string(&rem, "0");

    /*
     * x / 1
     */
    expect_from_cstr(&a, "1");

    CHECK(FOS_bignum_divide(&result, &rem, &b, &a));

    expect_string(
        &result,
        "123456789012345678901234567890"
    );
    expect_string(&rem, "0");

    FOS_bignum_free(&a);
    FOS_bignum_free(&b);
    FOS_bignum_free(&result);
    FOS_bignum_free(&rem);
}


/* ------------------------------------------------------------------------- */
/* Carry propagation                                                         */
/* ------------------------------------------------------------------------- */

static void test_addition_carries(void)
{
    puts("  addition carries");

    FOS_Bignum a;
    FOS_Bignum b;
    FOS_Bignum result;

    init_bignum(&a);
    init_bignum(&b);
    init_bignum(&result);

    expect_from_cstr(&a, "999999999999999999");
    expect_from_cstr(&b, "1");

    CHECK(FOS_bignum_add(&result, &a, &b));

    expect_string(&result, "1000000000000000000");

    /*
     * Carry through a much longer run of digits.
     */
    expect_from_cstr(
        &a,
        "999999999999999999999999999999999999999999999999"
    );

    expect_from_cstr(&b, "1");

    CHECK(FOS_bignum_add(&result, &a, &b));

    expect_string(
        &result,
        "1000000000000000000000000000000000000000000000000"
    );

    FOS_bignum_free(&a);
    FOS_bignum_free(&b);
    FOS_bignum_free(&result);
}


/* ------------------------------------------------------------------------- */
/* Borrow propagation                                                        */
/* ------------------------------------------------------------------------- */

static void test_subtraction_borrows(void)
{
    puts("  subtraction borrows");

    FOS_Bignum a;
    FOS_Bignum b;
    FOS_Bignum result;

    init_bignum(&a);
    init_bignum(&b);
    init_bignum(&result);

    expect_from_cstr(&a, "1000000000000000000");
    expect_from_cstr(&b, "1");

    CHECK(FOS_bignum_subtract(&result, &a, &b));

    expect_string(&result, "999999999999999999");

    /*
     * Long borrow chain.
     */
    expect_from_cstr(
        &a,
        "1000000000000000000000000000000000000000000000000"
    );

    expect_from_cstr(&b, "1");

    CHECK(FOS_bignum_subtract(&result, &a, &b));

    expect_string(
        &result,
        "999999999999999999999999999999999999999999999999"
    );

    /*
     * x - x = 0
     */
    CHECK(FOS_bignum_subtract(&result, &a, &a));
    expect_string(&result, "0");

    FOS_bignum_free(&a);
    FOS_bignum_free(&b);
    FOS_bignum_free(&result);
}


/* ------------------------------------------------------------------------- */
/* Multiplication                                                            */
/* ------------------------------------------------------------------------- */

static void test_multiplication(void)
{
    puts("  multiplication");

    FOS_Bignum a;
    FOS_Bignum b;
    FOS_Bignum result;

    init_bignum(&a);
    init_bignum(&b);
    init_bignum(&result);

    expect_from_cstr(&a, "359");
    expect_from_cstr(&b, "953");

    CHECK(FOS_bignum_multiply(&result, &a, &b));
    expect_string(&result, "342127");

    /*
     * Carry-heavy multiplication.
     */
    expect_from_cstr(&a, "999999999999999999");
    expect_from_cstr(&b, "999999999999999999");

    CHECK(FOS_bignum_multiply(&result, &a, &b));

    expect_string(
        &result,
        "999999999999999998000000000000000001"
    );

    /*
     * Self multiplication.
     */
    expect_from_cstr(&a, "89743527907");

    CHECK(FOS_bignum_multiply(&a, &a, &a));

    expect_string(
        &a,
        "8053900801194487800649"
    );

    FOS_bignum_free(&a);
    FOS_bignum_free(&b);
    FOS_bignum_free(&result);
}


/* ------------------------------------------------------------------------- */
/* Division                                                                  */
/* ------------------------------------------------------------------------- */

static void test_division(void)
{
    puts("  division");

    FOS_Bignum a;
    FOS_Bignum b;
    FOS_Bignum q;
    FOS_Bignum r;

    init_bignum(&a);
    init_bignum(&b);
    init_bignum(&q);
    init_bignum(&r);

    /*
     * General quotient/remainder case.
     */
    expect_from_cstr(
        &a,
        "476926509135632877643217"
    );

    expect_from_cstr(&b, "598367091");

    CHECK(FOS_bignum_divide(&q, &r, &a, &b));

    expect_string(&q, "797046689747904");
    expect_string(&r, "37815953");

    /*
     * Exact division.
     */
    expect_from_cstr(&a, "1000000000000000000");
    expect_from_cstr(&b, "10");

    CHECK(FOS_bignum_divide(&q, &r, &a, &b));

    expect_string(&q, "100000000000000000");
    expect_string(&r, "0");

    /*
     * Smaller dividend.
     */
    expect_from_cstr(&a, "123");
    expect_from_cstr(&b, "1000");

    CHECK(FOS_bignum_divide(&q, &r, &a, &b));

    expect_string(&q, "0");
    expect_string(&r, "123");

    /*
     * x / x = 1, remainder 0.
     */
    expect_from_cstr(
        &a,
        "123456789012345678901234567890"
    );

    CHECK(FOS_bignum_divide(&q, &r, &a, &a));

    expect_string(&q, "1");
    expect_string(&r, "0");

    FOS_bignum_free(&a);
    FOS_bignum_free(&b);
    FOS_bignum_free(&q);
    FOS_bignum_free(&r);
}


/* ------------------------------------------------------------------------- */
/* Aliasing of three-operand operations                                     */
/* ------------------------------------------------------------------------- */

static void test_arithmetic_aliasing(void)
{
    puts("  arithmetic aliasing");

    FOS_Bignum a;
    FOS_Bignum b;
    FOS_Bignum expected;

    init_bignum(&a);
    init_bignum(&b);
    init_bignum(&expected);

    /*
     * c = a + b, with c == a
     */
    expect_from_cstr(&a, "8974352790766543312807563498");
    expect_from_cstr(&b, "5983670913578976498342908");

    CHECK(FOS_bignum_add(&expected, &a, &b));
    CHECK(FOS_bignum_add(&a, &a, &b));

    expect_cmp(&a, &expected, 0);

    /*
     * c = a + b, with c == b
     */
    expect_from_cstr(&a, "12345678901234567890");
    expect_from_cstr(&b, "98765432109876543210");

    CHECK(FOS_bignum_add(&expected, &a, &b));
    CHECK(FOS_bignum_add(&b, &a, &b));

    expect_cmp(&b, &expected, 0);

    /*
     * c = a + a
     */
    expect_from_cstr(&a, "12345678901234567890");

    CHECK(FOS_bignum_add(&expected, &a, &a));
    CHECK(FOS_bignum_add(&a, &a, &a));

    expect_cmp(&a, &expected, 0);

    /*
     * c = a - b, with c == a
     */
    expect_from_cstr(&a, "10000000000000000000");
    expect_from_cstr(&b, "999999999");

    CHECK(FOS_bignum_subtract(&expected, &a, &b));
    CHECK(FOS_bignum_subtract(&a, &a, &b));

    expect_cmp(&a, &expected, 0);

    /*
     * c = a * b, with c == a
     */
    expect_from_cstr(&a, "123456789");
    expect_from_cstr(&b, "987654321");

    CHECK(FOS_bignum_multiply(&expected, &a, &b));
    CHECK(FOS_bignum_multiply(&a, &a, &b));

    expect_cmp(&a, &expected, 0);

    FOS_bignum_free(&a);
    FOS_bignum_free(&b);
    FOS_bignum_free(&expected);
}


/* ------------------------------------------------------------------------- */
/* Output buffer boundaries                                                  */
/* ------------------------------------------------------------------------- */

static void test_output_buffer(void)
{
    puts("  output buffer");

    FOS_Bignum bnum;
    init_bignum(&bnum);

    expect_from_cstr(&bnum, "123456789");

    /*
     * Nine digits + terminating NUL = 10 bytes.
     */
    char exact[10];

    CHECK(FOS_bignum_to_cstr(&bnum, exact, sizeof exact));
    CHECK(strcmp(exact, "123456789") == 0);

    /*
     * One byte too small must fail.
     */
    char too_small[9];

    CHECK(!FOS_bignum_to_cstr(
        &bnum,
        too_small,
        sizeof too_small
    ));

    /*
     * Plenty of space.
     */
    char large[64];

    CHECK(FOS_bignum_to_cstr(&bnum, large, sizeof large));
    CHECK(strcmp(large, "123456789") == 0);

    FOS_bignum_free(&bnum);
}


/* ------------------------------------------------------------------------- */
/* Repeated mutation                                                         */
/* ------------------------------------------------------------------------- */

static void test_repeated_mutation(void)
{
    puts("  repeated mutation");

    FOS_Bignum bnum;
    init_bignum(&bnum);

    /*
     * Repeated addition.
     */
    expect_from_cstr(&bnum, "0");

    for (unsigned i = 0; i < 10000; ++i)
        CHECK(FOS_bignum_add_u32(&bnum, 1));

    expect_string(&bnum, "10000");

    /*
     * Repeated multiplication/division by a small value.
     *
     * The division should restore the original value exactly.
     */
    expect_from_cstr(&bnum, "1");

    for (unsigned i = 0; i < 20; ++i)
        CHECK(FOS_bignum_mul_u32(&bnum, 10));

    expect_string(
        &bnum,
        "100000000000000000000"
    );

    for (unsigned i = 0; i < 20; ++i)
    {
        uint32_t rem = 0;

        CHECK(FOS_bignum_div_u32(&bnum, 10, &rem));
        CHECK(rem == 0);
    }

    expect_string(&bnum, "1");

    FOS_bignum_free(&bnum);
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(void)
{
    puts("FOS_Bignum test suite");
    puts("---------------------");

    test_construction();
    test_leading_zeros();
    test_copy();
    test_comparison();
    test_u32_arithmetic();
    test_zero_one();
    test_addition_carries();
    test_subtraction_borrows();
    test_multiplication();
    test_division();
    test_arithmetic_aliasing();
    test_output_buffer();
    test_repeated_mutation();

    puts("---------------------");
    puts("ALL TESTS PASSED");

    return EXIT_SUCCESS;
}
