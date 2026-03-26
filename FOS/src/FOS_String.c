#include "FOS_String.h"

FOS_String FOS_str_new(void)
{
    FOS_String fos_str = { 0 };

    fos_str.capacity = FOS_DEFAULT_CAPACITY;
    fos_str.str = calloc(fos_str.capacity, 1);

    if (fos_str.str == NULL)
        return (FOS_String) { 0 };

    return fos_str;
}

FOS_String FOS_str_new_cpc(size_t capacity)
{
    if (capacity == 0)
        return (FOS_String) { 0 };

    FOS_String new_str = { 0 };

    new_str.str = calloc(capacity, 1);

    if (new_str.str == NULL)
        return (FOS_String) { 0 };

    new_str.capacity = capacity;

    return new_str;
}

FOS_String FOS_str_copy(FOS_String src)
{
    if (src.str == NULL)
        return (FOS_String) { 0 };

    FOS_String cpy = { 0 };

    cpy.str = malloc(src.capacity);
    
    if (cpy.str == NULL)
        return (FOS_String) { 0 };

    memmove(cpy.str, src.str, src.size);

    cpy.size = src.size;
    cpy.capacity = src.capacity;

    return cpy;
}

FOS_String FOS_str_from_cstr(const char *cstr)
{
    if (cstr == NULL)
        return (FOS_String) { 0 };

    size_t len = strlen(cstr);

    FOS_String fos_str = { 0 };

    fos_str.capacity = len + 1;

    if (fos_str.capacity < FOS_DEFAULT_CAPACITY)
        fos_str.capacity = FOS_DEFAULT_CAPACITY;

    fos_str.str = malloc(fos_str.capacity);
    
    if (fos_str.str == NULL)
        return (FOS_String) { 0 };

    memcpy(fos_str.str, cstr, len + 1);

    fos_str.size = len;

    return fos_str;
}

char *FOS_str_cstr(FOS_String fos_str)
{
    if (fos_str.str == NULL)
        return NULL;

    char *cstr = calloc(fos_str.size + 1, 1);

    if (cstr == NULL)
        return NULL;
    
    memcpy(cstr, fos_str.str, fos_str.size);

    return cstr;
}

void FOS_str_free(FOS_String *fos_str) 
{
    if (fos_str == NULL)
        return;

    free(fos_str->str);

    fos_str->str = NULL;
    fos_str->size = 0;
    fos_str->capacity = 0;
}

FOS_Slice FOS_str_as_slice(FOS_String fos_str)
{
    if (fos_str.str == NULL)
        return (FOS_Slice) { 0 };

    return (FOS_Slice) { .data = fos_str.str, 
                         .size = fos_str.size };
}

FOS_Slice FOS_slice_from_cstr(const char *cstr)
{
    if (cstr == NULL)
        return (FOS_Slice) { 0 };

    return (FOS_Slice) { .data = cstr,
                         .size = strlen(cstr) };
}

FOS_Slice FOS_slice_sub(FOS_Slice fos_slc, size_t pos, size_t len)
{
    if (fos_slc.data == NULL || pos >= fos_slc.size) 
        return (FOS_Slice) { 0 };

    if (pos + len > fos_slc.size)
        len = fos_slc.size - pos;

    return (FOS_Slice) { .data = fos_slc.data + pos,
                         .size = len };
}

size_t FOS_slice_find_ch(FOS_Slice fos_slc, int ch)
{
    if (fos_slc.data == NULL)
        return SIZE_MAX;

    const char *found = memchr(fos_slc.data, ch, fos_slc.size);

    if (!found)
        return SIZE_MAX;

    return found - fos_slc.data;
}

size_t FOS_slice_find_subslice(FOS_Slice haystack, FOS_Slice needle)
{
    if (haystack.data == NULL || needle.data == NULL || needle.size == 0)
        return SIZE_MAX;

    if (haystack.size < needle.size)
        return SIZE_MAX;

    size_t *lps = malloc(needle.size * sizeof(size_t));

    if (!lps)
        return SIZE_MAX; // allocation failed

    // Build LPS (Longest Prefix Suffix) table
    lps[0] = 0;
    size_t len = 0;

    for (size_t i = 1; i < needle.size; ++i) 
    {
        while (len > 0 && needle.data[i] != needle.data[len])
            len = lps[len - 1];

        if (needle.data[i] == needle.data[len])
            ++len;

        lps[i] = len;
    }

    // KMP search
    size_t j = 0; // index in needle

    for (size_t i = 0; i < haystack.size; ++i) 
    {
        while (j > 0 && haystack.data[i] != needle.data[j])
            j = lps[j - 1];

        if (haystack.data[i] == needle.data[j])
            ++j;

        if (j == needle.size) 
        {
            free(lps);
            return i - needle.size + 1; // found at this position
        }
    }

    free(lps);
    return SIZE_MAX; // not found
}

size_t FOS_str_find_substr(FOS_String haystack, FOS_String needle)
{
    if (haystack.str == NULL || needle.str == NULL || needle.size == 0)
        return SIZE_MAX;

    if (haystack.size < needle.size)
        return SIZE_MAX;

    size_t *lps = malloc(needle.size * sizeof(size_t));

    if (lps == NULL)
        return SIZE_MAX; // allocation failed

    // Build LPS (Longest Prefix Suffix) table
    lps[0] = 0;
    size_t len = 0;

    for (size_t i = 1; i < needle.size; ++i) 
    {
        while (len > 0 && needle.str[i] != needle.str[len])
            len = lps[len - 1];

        if (needle.str[i] == needle.str[len])
            ++len;

        lps[i] = len;
    }

    // KMP search
    size_t j = 0; // index in needle

    for (size_t i = 0; i < haystack.size; ++i) 
    {
        while (j > 0 && haystack.str[i] != needle.str[j])
            j = lps[j - 1];

        if (haystack.str[i] == needle.str[j])
            ++j;

        if (j == needle.size) 
        {
            free(lps);
            return i - needle.size + 1; // found at this position
        }
    }

    free(lps);
    return SIZE_MAX; // not found
}

// Requires valid slices (.data != NULL)
int FOS_slice_cmp(FOS_Slice slc1, FOS_Slice slc2)
{
    size_t min_size = FOS_min_size(slc1.size, slc2.size);

    int res = memcmp(slc1.data, slc2.data, min_size);
    
    if (res == 0)
        return (slc1.size > slc2.size) - (slc1.size < slc2.size);

    return res;
}


static inline size_t FOS_min_size(size_t fst, size_t snd)
{
    return fst < snd ? fst : snd;
}

bool FOS_slice_valid(FOS_Slice fos_slc)
{
    return fos_slc.data != NULL;
}

bool FOS_string_valid(FOS_String fos_str)
{
    return fos_str.str != NULL;
}

FOS_String FOS_slice_to_str(FOS_Slice fos_slc)
{
    if (fos_slc.data == NULL)
        return (FOS_String) { 0 };

    if (fos_slc.size == SIZE_MAX)
        return (FOS_String) { 0 };

    FOS_String new_str = { 0 };

    new_str.capacity = fos_slc.size + 1;

    if (new_str.capacity < FOS_DEFAULT_CAPACITY)
        new_str.capacity = FOS_DEFAULT_CAPACITY;

    new_str.str = calloc(new_str.capacity, 1);
    
    if (new_str.str == NULL)
        return (FOS_String) { 0 };

    memcpy(new_str.str, fos_slc.data, fos_slc.size);

    new_str.size = fos_slc.size;

    return new_str;
}

bool FOS_str_grow(FOS_String *fos_str_ptr)
{
    if (fos_str_ptr == NULL || fos_str_ptr->str == NULL)
        return false;

    size_t new_capacity;

    if (fos_str_ptr->capacity == 0)
        new_capacity = FOS_DEFAULT_CAPACITY;
    else
    {
        if (fos_str_ptr->capacity > SIZE_MAX / 2)
            return false;

        new_capacity = fos_str_ptr->capacity * 2;
    }
    
    char *ptr = realloc(fos_str_ptr->str, new_capacity);

    if (ptr == NULL)
        return false;

    fos_str_ptr->str = ptr;
    fos_str_ptr->capacity = new_capacity;
    
    return true;
}

bool FOS_str_reserve(FOS_String *fos_str_ptr, size_t to_reserve)
{
    if (fos_str_ptr == NULL || fos_str_ptr->str == NULL || to_reserve == 0)
        return false;

    if (fos_str_ptr->capacity >= to_reserve)
        return true;

    if (fos_str_ptr->size + 1 > to_reserve)
        return false;

    if (fos_str_ptr->size == SIZE_MAX)
        return false;

    char *ptr = realloc(fos_str_ptr->str, to_reserve);

    if (ptr == NULL)
        return false;
    
    fos_str_ptr->str = ptr;
    fos_str_ptr->capacity = to_reserve;
    
    return true;
}

FOS_String FOS_str_append_slice(FOS_String fos_str, FOS_Slice tail)
{
    if (fos_str.str == NULL || tail.data == NULL)
        return (FOS_String) { 0 };

    FOS_String new_str = { 0 };
    size_t new_size = fos_str.size + tail.size;

    new_str.str = calloc(new_size + 1, 1);

    if (new_str.str == NULL)
        return (FOS_String) { 0 };

    new_str.capacity = new_size + 1;

    memmove(new_str.str, fos_str.str, fos_str.size);
    memmove(new_str.str + fos_str.size, tail.data, tail.size);

    new_str.size = new_size;

    return new_str;
}

FOS_String FOS_str_join(const FOS_Slice parts[], size_t count, FOS_Slice sep)
{
    if (count == 0)
        return (FOS_String) { 0 };

    if (sep.data == NULL)
        return (FOS_String) { 0 };

    size_t sep_len = sep.size;
    size_t total = 0;

    for (size_t i = 0; i < count; ++i)
    {
        if (parts[i].data == NULL)
            return (FOS_String) { 0 };
        
        total += parts[i].size;
    }

    if (total + sep_len * (count - 1) < total)
        return (FOS_String) { 0 }; // overflow

    total += sep_len * (count - 1);

    FOS_String new_str = FOS_str_new();

    if (!FOS_str_reserve(&new_str, total + 1))
        return (FOS_String) { 0 };

    size_t pos = 0;

    for (size_t i = 0; i < count; ++i)
    {
        memcpy(new_str.str + pos, parts[i].data, parts[i].size);
        pos += parts[i].size;

        if (i != count - 1)
        {
            memcpy(new_str.str + pos, sep.data, sep_len);
            pos += sep_len;
        }
    }

    new_str.str[pos] = '\0';
    new_str.size = pos;

    return new_str;
}

size_t FOS_slice_print(FOS_Slice fos_slc)
{
    if (fos_slc.data == NULL)
        return SIZE_MAX;

    size_t count = 0;

    for (size_t i = 0; i < fos_slc.size; ++i)
    {
        putchar(fos_slc.data[i]);
        ++count;
    }

    return count;
}

size_t FOS_slice_println(FOS_Slice fos_slc)
{
    size_t written = FOS_slice_print(fos_slc);

    putchar('\n');

    return written;
}

size_t FOS_str_print(FOS_String fos_str)
{
    if (fos_str.str == NULL)
        return SIZE_MAX;

    size_t count = 0;

    for (size_t i = 0; i < fos_str.size; ++i)
    {
        putchar(fos_str.str[i]);
        ++count;
    }

    return count;
}

size_t FOS_str_println(FOS_String fos_str)
{    
    size_t written = FOS_str_print(fos_str);

    putchar('\n');

    return written;
}

size_t FOS_slice_fprint(FOS_Slice fos_slc, FILE *stream)
{
    if (fos_slc.data == NULL || stream == NULL)
        return SIZE_MAX;

    size_t count = 0;

    for (size_t i = 0; i < fos_slc.size; ++i)
    {
        fputc(fos_slc.data[i], stream);
        ++count;
    }

    return count;
}

size_t FOS_slice_fprintln(FOS_Slice fos_slc, FILE *stream)
{
    size_t written = FOS_slice_fprint(fos_slc, stream);

    fputc('\n', stream);

    return written;
}

size_t FOS_str_fprint(FOS_String fos_str, FILE *stream)
{
    if (fos_str.str == NULL || stream == NULL)
        return SIZE_MAX;

    size_t count = 0;

    for (size_t i = 0; i < fos_str.size; ++i)
    {
        fputc(fos_str.str[i], stream);
        ++count;
    }

    return count;
}

size_t FOS_str_fprintln(FOS_String fos_str, FILE *stream)
{    
    size_t written = FOS_str_fprint(fos_str, stream);

    fputc('\n', stream);

    return written;
}

FOS_String FOS_str_concat(FOS_String s1, FOS_String s2)
{
    if (s1.str == NULL || s2.str == NULL)
        return (FOS_String) { 0 };

    size_t new_size = s1.size + s2.size;

    FOS_String new_str = { 0 };

    new_str.str = calloc(new_size + 1, 1);

    if (new_str.str == NULL)
        return (FOS_String) { 0 };

    new_str.capacity = new_size + 1;

    memcpy(new_str.str, s1.str, s1.size);
    memcpy(new_str.str + s1.size, s2.str, s2.size);

    new_str.size = new_size;

    return new_str;
}

FOS_String FOS_str_replace(FOS_String src, FOS_Slice needle, FOS_Slice replacement)
{
    if (src.str == NULL || needle.data == NULL || needle.size == 0 || replacement.data == NULL)
        return (FOS_String) { 0 };

    size_t index = FOS_slice_find_subslice(FOS_str_as_slice(src), needle);

    if (index == SIZE_MAX)
        return FOS_str_copy(src);

    size_t new_size = src.size - needle.size + replacement.size;

    FOS_String new_str = { 0 };
    
    new_str.str = calloc(new_size + 1, 1);

    if (new_str.str == NULL)
        return (FOS_String) { 0 };

    new_str.capacity = new_size + 1;

    memcpy(new_str.str, src.str, index);
    memcpy(new_str.str + index, replacement.data, replacement.size);
    memcpy(new_str.str + index + replacement.size, src.str + index + needle.size, src.size - index - needle.size);

    new_str.size = new_size; 

    return new_str;
}

FOS_SliceArray FOS_slice_split_ch(FOS_Slice src, int ch)
{
    if (src.data == NULL)
        return (FOS_SliceArray) { 0 };

    size_t count = 0;

    for (size_t i = 0; i < src.size; ++i)
        if (src.data[i] == ch)
            ++count;

    FOS_SliceArray slc_arr = { 0 };

    slc_arr.data = calloc(count + 1, sizeof(FOS_Slice));

    if (slc_arr.data == NULL)
        return (FOS_SliceArray) { 0 };

    size_t start = 0;
    size_t k = 0;

    for (size_t i = 0; i < src.size; ++i)
    {
        if (src.data[i] == ch)
        {
            slc_arr.data[k++] = FOS_slice_sub(src, start, i - start);
            start = i + 1;
        }
    }

    slc_arr.data[k++] = FOS_slice_sub(src, start, src.size - start);
    slc_arr.size = k;

    return slc_arr;
}

void FOS_slice_array_free(FOS_SliceArray *arr)
{
    if (arr == NULL)
        return;

    free(arr->data);

    arr->data = NULL;
    arr->size = 0;
}

FOS_String FOS_str_reverse(FOS_String fos_str)
{
    if (fos_str.str == NULL)
        return (FOS_String) { 0 };

    FOS_String new_str = { 0 };

    new_str.str = calloc(fos_str.size + 1, 1);

    if (new_str.str == NULL)
        return (FOS_String) { 0 };
    
    size_t i, j;

    for (i = fos_str.size - 1, j = 0; i > 0; --i, ++j)
        new_str.str[j] = fos_str.str[i];
    new_str.str[j] = fos_str.str[i];

    new_str.size = fos_str.size;
    new_str.capacity = fos_str.capacity;

    return new_str;
}

FOS_String FOS_str_trim(FOS_String fos_str)
{
    if (fos_str.str == NULL)
        return (FOS_String) { 0 };

    FOS_String new_str = FOS_str_copy(fos_str);

    if (new_str.str == NULL)
        return (FOS_String) { 0 };

    size_t spaces = 0;

    for (size_t i = 0; i < new_str.size && isspace((unsigned char)new_str.str[i]); ++i)
            ++spaces;

    if (spaces > 0)
    {
        memmove(new_str.str, new_str.str + spaces, new_str.size - spaces);
        new_str.size -= spaces;
        spaces = 0;
    }

    for (size_t j = new_str.size; j > 0 && isspace((unsigned char)new_str.str[j - 1]); --j)
            ++spaces;

    if (spaces > 0)
        new_str.size -= spaces;

    return new_str;
}

bool FOS_slice_starts_with(FOS_Slice s, FOS_Slice prefix)
{
    if (prefix.size > s.size)
        return false;

    return memcmp(s.data, prefix.data, prefix.size) == 0;
}

bool FOS_slice_ends_with(FOS_Slice s, FOS_Slice suffix)
{
    if (suffix.size > s.size)
        return false;

    return memcmp(s.data + (s.size - suffix.size), suffix.data, suffix.size) == 0;
}

FOS_Slice FOS_slice_trim(FOS_Slice s)
{
    size_t start = 0;
    size_t end = s.size;

    while (start < end && isspace((unsigned char)s.data[start]))
        ++start;

    while (end > start && isspace((unsigned char)s.data[end - 1]))
        --end;

    return (FOS_Slice) {
        .data = s.data + start,
        .size = end - start
    };
}

bool FOS_slice_eq(FOS_Slice a, FOS_Slice b)
{
    if (a.size != b.size)
        return false;

    return memcmp(a.data, b.data, a.size) == 0;
}

FOS_Slice FOS_slice_next_token_trimmed(FOS_Slice *input, int sep)
{
    if (input == NULL || !FOS_slice_valid(*input))
        return (FOS_Slice) { 0 };

    size_t start = 0;

    /* skip leading separators */
    while (start < input->size && input->data[start] == sep)
        ++start;

    if (start == input->size)
    {
        input->size = 0;
        return (FOS_Slice) { 0 };
    }

    size_t end = start;

    while (end < input->size && input->data[end] != sep)
        ++end;

    FOS_Slice token = { .data = input->data + start, .size = end - start };

    /* update the input slice */
    input->data += end;
    input->size -= end;

    /* trim whitespace from token */
    FOS_String temp = FOS_slice_to_str(token);
    temp = FOS_str_trim(temp);
    token = FOS_str_as_slice(temp); // creates a slice pointing to temp

    /* Note: temp must be freed after use, or stored in a pool if you reuse it */
    FOS_str_free(&temp);

    return token;
}
