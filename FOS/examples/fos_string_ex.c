// Compilation from MSYS2 UCRT64 shell:
// gcc fos_string_ex.c -I../include -L.. -lfos -o fos_string_ex

#include "FOS_String.h"
#include "FOS_Memory.h"
#include <stdio.h>

static void print_slice_array(FOS_SliceArray arr)
{
    for (size_t i = 0; i < arr.size; ++i)
    {
        printf("[%zu] \"", i);
        FOS_slice_print(arr.data[i]);
        printf("\"\n");
    }
}

int main(void)
{
    /*
     * FOS_String owns its character storage.
     * It can be created directly from a C string.
     */
    FOS_String greeting = FOS_str_from_cstr("Hello, FOS!");

    if (!FOS_string_valid(greeting))
    {
        fprintf(stderr, "Failed to create string\n");
        return 1;
    }

    printf("Greeting: ");
    FOS_str_println(greeting);

    printf("Size: %zu\n", greeting.size);
    printf("Capacity: %zu\n", greeting.capacity);

    /*
     * A FOS_Slice does not own its data.
     * This slice refers directly to greeting.
     */
    FOS_Slice greeting_slice = FOS_str_as_slice(greeting);

    printf("Slice: ");
    FOS_slice_println(greeting_slice);

    /*
     * A subslice also does not allocate or copy data.
     */
    FOS_Slice hello = FOS_slice_sub(greeting_slice, 0, 5);

    printf("First five characters: ");
    FOS_slice_println(hello);

    /*
     * FOS_slice_from_cstr() creates a non-owning slice
     * from an existing null-terminated C string.
     */
    FOS_Slice fos = FOS_slice_from_cstr("FOS");

    if (FOS_slice_starts_with(greeting_slice,
                              FOS_slice_from_cstr("Hello")))
    {
        printf("The greeting starts with \"Hello\"\n");
    }

    if (FOS_slice_ends_with(greeting_slice,
                            FOS_slice_from_cstr("!")))
    {
        printf("The greeting ends with '!'\n");
    }

    /*
     * Search functions return the zero-based position of the
     * first match, or SIZE_MAX when no match is found.
     */
    size_t pos = FOS_slice_find_subslice(greeting_slice, fos);

    if (pos != SIZE_MAX)
        printf("\"FOS\" found at position %zu\n", pos);

    /*
     * Convert a non-owning slice into an independently owned string.
     */
    FOS_String copied = FOS_slice_to_str(hello);

    if (FOS_string_valid(copied))
    {
        printf("Copied slice: ");
        FOS_str_println(copied);
    }

    /*
     * Create new strings by concatenating existing values.
     * The source strings are not modified.
     */
    FOS_String suffix = FOS_str_from_cstr(" Library");

    if (!FOS_string_valid(suffix))
    {
        FOS_str_free(&copied);
        FOS_str_free(&greeting);
        return 1;
    }

    FOS_String combined = FOS_str_concat(greeting, suffix);

    if (!FOS_string_valid(combined))
    {
        fprintf(stderr, "Failed to concatenate strings\n");
        FOS_str_free(&suffix);
        FOS_str_free(&copied);
        FOS_str_free(&greeting);
        return 1;
    }

    printf("Concatenated: ");
    FOS_str_println(combined);

    /*
     * Append a slice to an existing string.
     * This also returns a new owned string.
     */
    FOS_Slice period = FOS_slice_from_cstr(".");

    FOS_String sentence = FOS_str_append_slice(combined, period);

    if (!FOS_string_valid(sentence))
    {
        fprintf(stderr, "Failed to append slice\n");
        FOS_str_free(&combined);
        FOS_str_free(&suffix);
        FOS_str_free(&copied);
        FOS_str_free(&greeting);
        return 1;
    }

    printf("Appended: ");
    FOS_str_println(sentence);

    /*
     * Replace the first occurrence of a substring.
     */
    FOS_String replaced = FOS_str_replace(
        sentence,
        FOS_slice_from_cstr("FOS"),
        FOS_slice_from_cstr("Fission Of Soul")
    );

    if (FOS_string_valid(replaced))
    {
        printf("Replaced: ");
        FOS_str_println(replaced);
    }

    /*
     * Create a reversed copy.
     */
    FOS_String reversed = FOS_str_reverse(greeting);

    if (FOS_string_valid(reversed))
    {
        printf("Reversed: ");
        FOS_str_println(reversed);
    }

    /*
     * Trim whitespace from both ends.
     */
    FOS_String spaced = FOS_str_from_cstr("   hello world   ");

    if (FOS_string_valid(spaced))
    {
        FOS_String trimmed = FOS_str_trim(spaced);

        if (FOS_string_valid(trimmed))
        {
            printf("Trimmed: \"");
            FOS_str_print(trimmed);
            printf("\"\n");

            FOS_str_free(&trimmed);
        }

        /*
         * FOS_slice_trim() performs the same operation without
         * copying the character data.
         */
        FOS_Slice spaced_slice = FOS_str_as_slice(spaced);
        FOS_Slice trimmed_slice = FOS_slice_trim(spaced_slice);

        printf("Trimmed slice: \"");
        FOS_slice_print(trimmed_slice);
        printf("\"\n");

        FOS_str_free(&spaced);
    }

    /*
     * Reserve additional capacity without changing the string's
     * logical contents.
     */
    if (FOS_str_reserve(&greeting, 128))
    {
        printf("Greeting capacity after reserve: %zu\n",
               greeting.capacity);
    }

    /*
     * Split a slice into non-owning slices.
     */
    FOS_String csv = FOS_str_from_cstr("red,green,blue,yellow");

    if (FOS_string_valid(csv))
    {
        FOS_SliceArray parts = FOS_slice_split_ch(
            FOS_str_as_slice(csv),
            ','
        );

        if (parts.data != NULL)
        {
            printf("Split fields:\n");
            print_slice_array(parts);

            /*
             * Only the array of slice descriptors is owned by
             * FOS_SliceArray. The character data still belongs
             * to csv.
             */
            FOS_slice_array_free(&parts);
        }

        FOS_str_free(&csv);
    }

    /*
     * Tokenize a slice incrementally.
     *
     * The input slice itself is advanced after each call.
     */
    FOS_String line = FOS_str_from_cstr(
        "  first ; second ;   third   ; fourth  "
    );

    if (FOS_string_valid(line))
    {
        FOS_Slice input = FOS_str_as_slice(line);

        printf("Tokens:\n");

        while (FOS_slice_valid(input))
        {
            FOS_Slice token = FOS_slice_next_token_trimmed(&input, ';');

            if (!FOS_slice_valid(token))
                break;

            printf("  \"");
            FOS_slice_print(token);
            printf("\"\n");
        }

        FOS_str_free(&line);
    }

    /*
     * FOS_str_cstr() creates a new allocation.
     * The caller owns that allocation and must release it
     * through the FOS allocator.
     */
    char *cstr = FOS_str_cstr(greeting);

    if (cstr != NULL)
    {
        printf("As C string: %s\n", cstr);
        FOS_free(cstr);
    }

    /*
     * Compare slices lexicographically.
     */
    FOS_Slice apple = FOS_slice_from_cstr("apple");
    FOS_Slice banana = FOS_slice_from_cstr("banana");

    int cmp = FOS_slice_cmp(apple, banana);

    if (cmp < 0)
        printf("\"apple\" comes before \"banana\"\n");
    else if (cmp > 0)
        printf("\"apple\" comes after \"banana\"\n");
    else
        printf("\"apple\" and \"banana\" are equal\n");

    /*
     * Release all owned strings.
     *
     * Slices require no deallocation because they do not own
     * their character data.
     */
    FOS_str_free(&replaced);
    FOS_str_free(&reversed);
    FOS_str_free(&sentence);
    FOS_str_free(&combined);
    FOS_str_free(&suffix);
    FOS_str_free(&copied);
    FOS_str_free(&greeting);

    return 0;
}
