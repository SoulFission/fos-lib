#include "FOS_String.h"
#include "FOS_Memory.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#define ASSERT_STRING(s, expected)                                      \
    do {                                                                \
        assert(FOS_string_valid(s));                                    \
        assert((s).size == strlen(expected));                           \
        assert(memcmp((s).str, (expected), (s).size) == 0);             \
        assert((s).str[(s).size] == '\0');                              \
    } while (0)

#define ASSERT_SLICE(s, expected)                                      \
    do {                                                               \
        assert(FOS_slice_valid(s));                                    \
        assert((s).size == strlen(expected));                          \
        assert(memcmp((s).data, (expected), (s).size) == 0);            \
    } while (0)


static void test_constructors(void)
{
    FOS_String s;

    s = FOS_str_new();
    assert(FOS_string_valid(s));
    assert(s.size == 0);
    assert(s.capacity >= 1);
    assert(s.str[0] == '\0');
    FOS_str_free(&s);

    s = FOS_str_new_cpc(32);
    assert(FOS_string_valid(s));
    assert(s.size == 0);
    assert(s.capacity == 32);
    assert(s.str[0] == '\0');
    FOS_str_free(&s);

    s = FOS_str_new_cpc(0);
    assert(!FOS_string_valid(s));

    s = FOS_str_from_cstr("hello world");
    ASSERT_STRING(s, "hello world");
    FOS_str_free(&s);

    s = FOS_str_from_cstr("");
    ASSERT_STRING(s, "");
    FOS_str_free(&s);

    s = FOS_str_from_cstr(NULL);
    assert(!FOS_string_valid(s));
}


static void test_copy(void)
{
    FOS_String src = FOS_str_from_cstr("hello world");
    FOS_String copy = FOS_str_copy(src);

    ASSERT_STRING(src, "hello world");
    ASSERT_STRING(copy, "hello world");

    assert(copy.str != src.str);

    copy.str[0] = 'H';

    ASSERT_STRING(src, "hello world");
    ASSERT_STRING(copy, "Hello world");

    FOS_str_free(&copy);
    FOS_str_free(&src);

    src = FOS_str_new();
    copy = FOS_str_copy(src);

    ASSERT_STRING(src, "");
    ASSERT_STRING(copy, "");

    FOS_str_free(&copy);
    FOS_str_free(&src);

    copy = FOS_str_copy((FOS_String){ 0 });
    assert(!FOS_string_valid(copy));
}


static void test_cstr_conversion(void)
{
    FOS_String s = FOS_str_from_cstr("hello");

    char *p = FOS_str_cstr(s);

    assert(p != NULL);
    assert(strcmp(p, "hello") == 0);

    p[0] = 'H';

    ASSERT_STRING(s, "hello");

    FOS_free(p);
    FOS_str_free(&s);

    assert(FOS_str_cstr((FOS_String){ 0 }) == NULL);
}


static void test_slices(void)
{
    const char text[] = "hello world";

    FOS_Slice s = FOS_slice_from_cstr(text);

    ASSERT_SLICE(s, "hello world");

    FOS_Slice sub = FOS_slice_sub(s, 0, 5);
    ASSERT_SLICE(sub, "hello");

    sub = FOS_slice_sub(s, 6, 5);
    ASSERT_SLICE(sub, "world");

    sub = FOS_slice_sub(s, 6, 100);
    ASSERT_SLICE(sub, "world");

    sub = FOS_slice_sub(s, s.size, 0);
    assert(FOS_slice_valid(sub));
    assert(sub.size == 0);

    sub = FOS_slice_sub(s, s.size + 1, 0);
    assert(!FOS_slice_valid(sub));

    assert(!FOS_slice_valid(FOS_slice_from_cstr(NULL)));

    FOS_String owned = FOS_str_from_cstr("abcdef");
    FOS_Slice owned_slice = FOS_str_as_slice(owned);

    ASSERT_SLICE(owned_slice, "abcdef");

    FOS_str_free(&owned);
}


static void test_slice_search(void)
{
    FOS_Slice s = FOS_slice_from_cstr("abracadabra");

    assert(FOS_slice_find_ch(s, 'a') == 0);
    assert(FOS_slice_find_ch(s, 'c') == 4);
    assert(FOS_slice_find_ch(s, 'z') == SIZE_MAX);

    assert(FOS_slice_find_subslice(
        s, FOS_slice_from_cstr("abra")) == 0);

    assert(FOS_slice_find_subslice(
        s, FOS_slice_from_cstr("cada")) == 4);

    assert(FOS_slice_find_subslice(
        s, FOS_slice_from_cstr("xyz")) == SIZE_MAX);

    assert(FOS_slice_find_subslice(
        s, FOS_slice_from_cstr("")) == SIZE_MAX);

    assert(FOS_slice_find_subslice(
        (FOS_Slice){ 0 }, s) == SIZE_MAX);
}


static void test_slice_comparison(void)
{
    FOS_Slice abc1 = FOS_slice_from_cstr("abc");
    FOS_Slice abc2 = FOS_slice_from_cstr("abc");
    FOS_Slice abd  = FOS_slice_from_cstr("abd");
    FOS_Slice ab   = FOS_slice_from_cstr("ab");

    assert(FOS_slice_cmp(abc1, abc2) == 0);
    assert(FOS_slice_cmp(abc1, abd) < 0);
    assert(FOS_slice_cmp(abd, abc1) > 0);
    assert(FOS_slice_cmp(ab, abc1) < 0);
    assert(FOS_slice_cmp(abc1, ab) > 0);

    assert(FOS_slice_eq(abc1, abc2));
    assert(!FOS_slice_eq(abc1, abd));
    assert(!FOS_slice_eq(abc1, ab));

    assert(FOS_slice_starts_with(
        abc1, FOS_slice_from_cstr("a")));

    assert(FOS_slice_starts_with(
        abc1, FOS_slice_from_cstr("abc")));

    assert(!FOS_slice_starts_with(
        abc1, FOS_slice_from_cstr("b")));

    assert(FOS_slice_ends_with(
        abc1, FOS_slice_from_cstr("c")));

    assert(FOS_slice_ends_with(
        abc1, FOS_slice_from_cstr("abc")));

    assert(!FOS_slice_ends_with(
        abc1, FOS_slice_from_cstr("b")));

    FOS_Slice empty = {
        .data = abc1.data,
        .size = 0
    };

    assert(FOS_slice_starts_with(abc1, empty));
    assert(FOS_slice_ends_with(abc1, empty));
    assert(FOS_slice_eq(empty, empty));

    assert(!FOS_slice_eq(abc1, (FOS_Slice){ 0 }));
    assert(!FOS_slice_starts_with(abc1, (FOS_Slice){ 0 }));
    assert(!FOS_slice_ends_with(abc1, (FOS_Slice){ 0 }));
}


static void test_string_search(void)
{
    FOS_String haystack =
        FOS_str_from_cstr("There's no place like home.");

    FOS_String needle =
        FOS_str_from_cstr("place");

    FOS_String absent =
        FOS_str_from_cstr("Damn");

    assert(FOS_str_find_substr(haystack, needle) == 11);
    assert(FOS_str_find_substr(haystack, absent) == SIZE_MAX);

    FOS_str_free(&absent);
    FOS_str_free(&needle);
    FOS_str_free(&haystack);
}


static void test_grow_reserve(void)
{
    FOS_String s = FOS_str_from_cstr("hello");

    size_t old_capacity = s.capacity;

    assert(FOS_str_grow(&s));
    assert(s.capacity == old_capacity * 2);
    ASSERT_STRING(s, "hello");

    assert(FOS_str_reserve(&s, s.capacity));
    ASSERT_STRING(s, "hello");

    assert(FOS_str_reserve(&s, 128));
    assert(s.capacity >= 128);
    ASSERT_STRING(s, "hello");

    assert(!FOS_str_reserve(&s, 0));
    ASSERT_STRING(s, "hello");

    FOS_str_free(&s);

    assert(!FOS_str_grow(NULL));
    assert(!FOS_str_reserve(NULL, 10));

    FOS_String invalid = { 0 };

    assert(!FOS_str_grow(&invalid));
    assert(!FOS_str_reserve(&invalid, 10));

    FOS_String boundary = {
        .str = (char *)1,
        .size = SIZE_MAX,
        .capacity = SIZE_MAX
    };

    assert(!FOS_str_reserve(&boundary, SIZE_MAX));
}


static void test_append_concat(void)
{
    FOS_String a = FOS_str_from_cstr("hello");
    FOS_String b = FOS_str_from_cstr(" world");

    FOS_String concat = FOS_str_concat(a, b);

    ASSERT_STRING(a, "hello");
    ASSERT_STRING(b, " world");
    ASSERT_STRING(concat, "hello world");

    FOS_String append = FOS_str_append_slice(
        a, FOS_slice_from_cstr(" world"));

    ASSERT_STRING(a, "hello");
    ASSERT_STRING(append, "hello world");

    FOS_str_free(&append);
    FOS_str_free(&concat);
    FOS_str_free(&b);
    FOS_str_free(&a);
}


static void test_join(void)
{
    FOS_Slice parts[] = {
        FOS_slice_from_cstr("one"),
        FOS_slice_from_cstr("two"),
        FOS_slice_from_cstr("three")
    };

    FOS_String joined = FOS_str_join(
        parts,
        3,
        FOS_slice_from_cstr(", "));

    ASSERT_STRING(joined, "one, two, three");

    FOS_str_free(&joined);

    FOS_String no_separator = FOS_str_join(
        parts,
        3,
        (FOS_Slice){ .data = "x", .size = 0 });

    ASSERT_STRING(no_separator, "onetwothree");

    FOS_str_free(&no_separator);

    assert(!FOS_string_valid(
        FOS_str_join(NULL, 3,
                     FOS_slice_from_cstr(", "))));

    assert(!FOS_string_valid(
        FOS_str_join(parts, 0,
                     FOS_slice_from_cstr(", "))));
}


static void test_replace(void)
{
    FOS_String src =
        FOS_str_from_cstr("one fish, two fish");

    FOS_String replaced = FOS_str_replace(
        src,
        FOS_slice_from_cstr("fish"),
        FOS_slice_from_cstr("cat"));

    ASSERT_STRING(src, "one fish, two fish");
    ASSERT_STRING(replaced, "one cat, two fish");

    FOS_str_free(&replaced);

    /*
     * No match: result should be a proper independent copy.
     */
    replaced = FOS_str_replace(
        src,
        FOS_slice_from_cstr("dog"),
        FOS_slice_from_cstr("cat"));

    ASSERT_STRING(src, "one fish, two fish");
    ASSERT_STRING(replaced, "one fish, two fish");

    assert(replaced.str != src.str);

    FOS_str_free(&replaced);

    /*
     * Empty replacement.
     */
    replaced = FOS_str_replace(
        src,
        FOS_slice_from_cstr("fish"),
        (FOS_Slice){ .data = "x", .size = 0 });

    ASSERT_STRING(replaced, "one , two fish");

    FOS_str_free(&replaced);
    FOS_str_free(&src);
}


static void test_split(void)
{
    FOS_Slice src =
        FOS_slice_from_cstr(",one,,two,three,");

    FOS_SliceArray arr =
        FOS_slice_split_ch(src, ',');

    assert(arr.data != NULL);
    assert(arr.size == 6);

    ASSERT_SLICE(arr.data[0], "");
    ASSERT_SLICE(arr.data[1], "one");
    ASSERT_SLICE(arr.data[2], "");
    ASSERT_SLICE(arr.data[3], "two");
    ASSERT_SLICE(arr.data[4], "three");
    ASSERT_SLICE(arr.data[5], "");

    FOS_slice_array_free(&arr);

    assert(arr.data == NULL);
    assert(arr.size == 0);

    arr = FOS_slice_split_ch(
        FOS_slice_from_cstr(""), ',');

    assert(arr.data != NULL);
    assert(arr.size == 1);
    ASSERT_SLICE(arr.data[0], "");

    FOS_slice_array_free(&arr);

    arr = FOS_slice_split_ch(
        (FOS_Slice){ 0 }, ',');

    assert(arr.data == NULL);
    assert(arr.size == 0);
}


static void test_reverse(void)
{
    FOS_String s = FOS_str_from_cstr("abcdef");
    FOS_String r = FOS_str_reverse(s);

    ASSERT_STRING(s, "abcdef");
    ASSERT_STRING(r, "fedcba");

    FOS_str_free(&r);
    FOS_str_free(&s);

    s = FOS_str_from_cstr("");
    r = FOS_str_reverse(s);

    ASSERT_STRING(r, "");

    FOS_str_free(&r);
    FOS_str_free(&s);

    r = FOS_str_reverse((FOS_String){ 0 });
    assert(!FOS_string_valid(r));
}


static void test_trim(void)
{
    FOS_String s =
        FOS_str_from_cstr("  \t hello world \r\n ");

    FOS_String trimmed = FOS_str_trim(s);

    ASSERT_STRING(s, "  \t hello world \r\n ");
    ASSERT_STRING(trimmed, "hello world");

    assert(trimmed.str != s.str);

    FOS_str_free(&trimmed);
    FOS_str_free(&s);

    s = FOS_str_from_cstr("   \t\r\n ");
    trimmed = FOS_str_trim(s);

    ASSERT_STRING(trimmed, "");

    FOS_str_free(&trimmed);
    FOS_str_free(&s);
}


static void test_slice_trim(void)
{
    const char text[] = "\t  hello world \r\n";

    FOS_Slice s = FOS_slice_from_cstr(text);
    FOS_Slice trimmed = FOS_slice_trim(s);

    ASSERT_SLICE(trimmed, "hello world");

    trimmed = FOS_slice_trim(
        FOS_slice_from_cstr("   "));

    ASSERT_SLICE(trimmed, "");

    trimmed = FOS_slice_trim(
        FOS_slice_from_cstr(""));

    ASSERT_SLICE(trimmed, "");

    trimmed = FOS_slice_trim((FOS_Slice){ 0 });
    assert(!FOS_slice_valid(trimmed));
}


static void test_tokenization(void)
{
    FOS_Slice input =
        FOS_slice_from_cstr("  one , two , three ");

    FOS_Slice token =
        FOS_slice_next_token_trimmed(&input, ',');

    ASSERT_SLICE(token, "one");

    token =
        FOS_slice_next_token_trimmed(&input, ',');

    ASSERT_SLICE(token, "two");

    token =
        FOS_slice_next_token_trimmed(&input, ',');

    ASSERT_SLICE(token, "three");

    token =
        FOS_slice_next_token_trimmed(&input, ',');

    assert(!FOS_slice_valid(token));
    assert(input.size == 0);

    input = FOS_slice_from_cstr(",,,");
    token =
        FOS_slice_next_token_trimmed(&input, ',');

    assert(!FOS_slice_valid(token));
    assert(input.size == 0);

    assert(!FOS_slice_valid(
        FOS_slice_next_token_trimmed(NULL, ',')));
}


static void test_binary_data(void)
{
    const char raw[] = {
        'a', '\0', 'b', 'c', '\0'
    };

    FOS_Slice s = {
        .data = raw,
        .size = 4
    };

    assert(FOS_slice_find_ch(s, '\0') == 1);
    assert(FOS_slice_find_ch(s, 'b') == 2);

    FOS_Slice needle = {
        .data = raw + 1,
        .size = 3
    };

    assert(FOS_slice_find_subslice(s, needle) == 1);

    FOS_String converted = FOS_slice_to_str(s);

    assert(FOS_string_valid(converted));
    assert(converted.size == 4);
    assert(memcmp(converted.str, raw, 4) == 0);
    assert(converted.str[4] == '\0');

    FOS_str_free(&converted);
}


static void test_printing(void)
{
    FOS_String s = FOS_str_from_cstr("hello");
    FOS_Slice sl = FOS_slice_from_cstr("world");

    FILE *f = tmpfile();
    assert(f != NULL);

    assert(FOS_str_fprint(s, f) == 5);
    assert(FOS_slice_fprint(sl, f) == 5);
    assert(FOS_str_fprintln(s, f) == 6);
    assert(FOS_slice_fprintln(sl, f) == 6);

    rewind(f);

    char buffer[32] = { 0 };

    size_t n = fread(buffer, 1, sizeof(buffer), f);

    assert(n == strlen("helloworldhello\nworld\n"));
    assert(memcmp(
        buffer,
        "helloworldhello\nworld\n",
        n) == 0);

    fclose(f);

    FOS_str_free(&s);
}


int main(void)
{
    puts("Running FOS_String tests...");

    test_constructors();
    test_copy();
    test_cstr_conversion();
    test_slices();
    test_slice_search();
    test_slice_comparison();
    test_string_search();
    test_grow_reserve();
    test_append_concat();
    test_join();
    test_replace();
    test_split();
    test_reverse();
    test_trim();
    test_slice_trim();
    test_tokenization();
    test_binary_data();
    test_printing();

    puts("All tests passed.");
    return 0;
}
