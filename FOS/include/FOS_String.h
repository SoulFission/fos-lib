#ifndef FOS_STRING_H
#define FOS_STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>

static const size_t FOS_DEFAULT_CAPACITY = 64;

typedef struct {
    char *str;
    size_t size;
    size_t capacity;
} FOS_String;

typedef struct {
    const char *data;
    size_t size;
} FOS_Slice;

typedef struct {
    FOS_Slice *data;
    size_t size;
} FOS_SliceArray;

FOS_String FOS_str_new(void);
FOS_String FOS_str_new_cpc(size_t capacity);
FOS_String FOS_str_copy(FOS_String src);
FOS_String FOS_str_from_cstr(const char *cstr);
char *FOS_str_cstr(FOS_String s);
void FOS_str_free(FOS_String *fos_str);
FOS_Slice FOS_str_as_slice(FOS_String fos_str);
FOS_Slice FOS_slice_from_cstr(const char *cstr);
FOS_Slice FOS_slice_sub(FOS_Slice fos_slc, size_t pos, size_t len);
size_t FOS_slice_find_ch(FOS_Slice fos_slc, int ch);
size_t FOS_slice_find_subslice(FOS_Slice haystack, FOS_Slice needle);
size_t FOS_str_find_substr(FOS_String haystack, FOS_String needle);
int FOS_slice_cmp(FOS_Slice a, FOS_Slice b);
static inline size_t FOS_min_size(size_t fst, size_t snd);
bool FOS_slice_valid(FOS_Slice fos_slc);
bool FOS_string_valid(FOS_String fos_str);
FOS_String FOS_slice_to_str(FOS_Slice fos_slc);
bool FOS_str_grow(FOS_String *fos_str_ptr);
bool FOS_str_reserve(FOS_String *fos_str_ptr, size_t to_reserve);
FOS_String FOS_str_append_slice(FOS_String fos_str, FOS_Slice tail);
FOS_String FOS_str_join(const FOS_Slice parts[], size_t count, FOS_Slice sep);
size_t FOS_slice_print(FOS_Slice fos_slc);
size_t FOS_slice_println(FOS_Slice fos_slc);
size_t FOS_str_print(FOS_String fos_str);
size_t FOS_str_println(FOS_String fos_str);
size_t FOS_slice_fprint(FOS_Slice s, FILE *stream);
size_t FOS_slice_fprintln(FOS_Slice fos_slc, FILE *stream);
size_t FOS_str_fprint(FOS_String fos_str, FILE *stream);
size_t FOS_str_fprintln(FOS_String fos_str, FILE *stream);
FOS_String FOS_str_concat(FOS_String s1, FOS_String s2);
FOS_String FOS_str_replace(FOS_String src, FOS_Slice needle, FOS_Slice replacement);
FOS_SliceArray FOS_slice_split_ch(FOS_Slice src, int ch);
void FOS_slice_array_free(FOS_SliceArray *arr);
FOS_String FOS_str_reverse(FOS_String fos_str);
FOS_String FOS_str_trim(FOS_String fos_str);
bool FOS_slice_starts_with(FOS_Slice s, FOS_Slice prefix);
bool FOS_slice_ends_with(FOS_Slice s, FOS_Slice suffix);
FOS_Slice FOS_slice_trim(FOS_Slice s);
bool FOS_slice_eq(FOS_Slice a, FOS_Slice b);
FOS_Slice FOS_slice_next_token_trimmed(FOS_Slice *input, int sep);

#endif
