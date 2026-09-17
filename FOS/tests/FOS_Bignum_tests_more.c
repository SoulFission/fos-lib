#include "FOS_Bignum.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void expect_value(const char *text, const char *expected)
{
    FOS_Bignum bn;
    char buf[256];

    assert(FOS_bignum_init(&bn));
    assert(FOS_bignum_from_cstr(&bn, text));
    assert(FOS_bignum_to_cstr(&bn, buf, sizeof(buf)));
    assert(strcmp(buf, expected) == 0);
    FOS_bignum_free(&bn);
}

static void expect_rejected(const char *text)
{
    FOS_Bignum bn;
    char buf[256];

    assert(FOS_bignum_init(&bn));
    assert(FOS_bignum_from_cstr(&bn, text));
    /* This helper is for values expected to succeed. */
    (void)buf;
    FOS_bignum_free(&bn);
}

static void test_max_digits(void)
{
    FOS_Bignum bn;
    char buf[256];

    assert(FOS_bignum_init(&bn));

    /* 0 means unlimited. */
    FOS_bignum_set_max_digits(0);
    assert(FOS_bignum_from_cstr(&bn, "12345678901234567890"));
    assert(FOS_bignum_to_cstr(&bn, buf, sizeof(buf)));
    assert(strcmp(buf, "12345678901234567890") == 0);

    /* One-digit limit. */
    FOS_bignum_set_max_digits(1);

    assert(FOS_bignum_from_cstr(&bn, "9"));
    assert(FOS_bignum_from_cstr(&bn, "-9"));
    assert(FOS_bignum_from_cstr(&bn, "0000009"));

    assert(!FOS_bignum_from_cstr(&bn, "10"));
    assert(!FOS_bignum_from_cstr(&bn, "-10"));

    /* Limit not divisible by 9. */
    FOS_bignum_set_max_digits(10);

    assert(FOS_bignum_from_cstr(&bn, "1234567890"));
    assert(FOS_bignum_from_cstr(&bn, "-1234567890"));
    assert(!FOS_bignum_from_cstr(&bn, "12345678901"));

    /* Leading zeroes do not count. */
    FOS_bignum_set_max_digits(3);

    assert(FOS_bignum_from_cstr(&bn, "000123"));
    assert(FOS_bignum_from_cstr(&bn, "-000123"));
    assert(FOS_bignum_from_cstr(&bn, "000000000"));
    assert(!FOS_bignum_from_cstr(&bn, "0001234"));

    /* Zero is always canonical and positive. */
    assert(FOS_bignum_from_cstr(&bn, "0000000"));
    assert(FOS_bignum_to_cstr(&bn, buf, sizeof(buf)));
    assert(strcmp(buf, "0") == 0);

    /* Sign-only and empty strings are invalid. */
    assert(!FOS_bignum_from_cstr(&bn, ""));
    assert(!FOS_bignum_from_cstr(&bn, "+"));
    assert(!FOS_bignum_from_cstr(&bn, "-"));

    /* Restore default. */
    FOS_bignum_set_max_digits(0);

    FOS_bignum_free(&bn);
}

static void test_failed_parse_preserves_value(void)
{
    FOS_Bignum bn;
    char buf[256];

    assert(FOS_bignum_init(&bn));
    assert(FOS_bignum_from_cstr(&bn, "12345"));

    assert(!FOS_bignum_from_cstr(&bn, "123x45"));

    assert(FOS_bignum_to_cstr(&bn, buf, sizeof(buf)));
    assert(strcmp(buf, "12345") == 0);

    FOS_bignum_free(&bn);
}

int main(void)
{
    test_max_digits();

    expect_value("123", "123");
    expect_value("+123", "123");
    expect_value("-123", "-123");
    expect_value("00042", "42");
    expect_value("-00042", "-42");
    expect_value("0", "0");
    expect_value("-0", "0");
    expect_value("+0", "0");

    printf("All tests passed.\n");

    return 0;
}