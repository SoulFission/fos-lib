#include "FOS_Bignum.h"

bool FOS_bignum_init(FOS_Bignum *bn)
{
    if (bn == NULL)
        return false;

    bn->digits = NULL;
    bn->size = 0;
    bn->capacity = 0;
    bn->sign = +1;

    return true;
}

void FOS_bignum_set_max_digits(size_t max)
{
    // 0 means "no limit"
    fos_bignum_max_digits = max;
}

void FOS_bignum_free(FOS_Bignum *bn)
{
    if (bn == NULL)
        return;

    free(bn->digits);

    bn->digits = NULL;
    bn->size = 0;
    bn->capacity = 0;
    bn->sign = +1;
}

bool FOS_bignum_reserve(FOS_Bignum *bn, size_t cap)
{
    if (bn == NULL)
        return false;

    if (fos_bignum_max_digits != 0 && cap > fos_bignum_max_digits)
        return false;

    if (cap <= bn->capacity)
        return true;

    uint32_t *tmp = realloc(bn->digits, cap * sizeof(uint32_t));
    
    if (tmp == NULL)
        return false;

    bn->digits = tmp;
    bn->capacity = cap;

    return true;
}

bool FOS_bignum_trim(FOS_Bignum *bn)
{
    if (bn == NULL)
        return false;

    while (bn->size > 1 && bn->digits[bn->size - 1] == 0)
        bn->size--;

    if (bn->size == 1 && bn->digits[0] == 0)
        bn->sign = +1;

    return true;
}

bool FOS_bignum_copy(FOS_Bignum *dst, const FOS_Bignum *src)
{
    if (dst == NULL || src == NULL)
        return false;

    if (src->size > 0 && src->digits == NULL)
        return false;

    if (dst->capacity < src->size)
    {
        if (!FOS_bignum_reserve(dst, src->size))
            return false;
    }

    memcpy(dst->digits, src->digits, src->size * sizeof(uint32_t));

    dst->size = src->size;
    dst->sign = src->sign;

    return true;
}


bool FOS_bignum_from_u64(FOS_Bignum *bn, uint64_t value)
{
    if (bn == NULL)
        return false;

    bn->size = 0;

    while (value > 0)
    {
        if (bn->size >= bn->capacity)
        {
            size_t new_cap = bn->capacity ? bn->capacity * 2 : 8;

            if (!FOS_bignum_reserve(bn, new_cap))
                return false;
        }

        bn->digits[bn->size++] = value % FOS_BIGNUM_BASE;
        value /= FOS_BIGNUM_BASE;
    }

    if (bn->size == 0)
    {
        if (bn->capacity == 0)
        {
            if (!FOS_bignum_reserve(bn, 1))
                return false;
        }

        bn->digits[0] = 0;
        bn->size = 1;
    }

    bn->sign = +1;

    return true;
}

int FOS_bignum_cmp_abs(const FOS_Bignum *fst, const FOS_Bignum *snd)
{
    if (fst->size < snd->size)
        return -1;
    else if (fst->size > snd->size)
        return 1;
    else // fst->size == snd->size
    {
        for (size_t i = fst->size; i-- > 0;)
        {
            if (fst->digits[i] < snd->digits[i])
                return -1;
            else if (fst->digits[i] > snd->digits[i])
                return 1;
        }
    }

    return 0;
}

int FOS_bignum_cmp(const FOS_Bignum *fst, const FOS_Bignum *snd)
{
    if (fst->sign == -1 && snd->sign == +1)
        return -1;
    
    if (fst->sign == +1 && snd->sign == -1)
        return 1;

    bool neg = false;

    if (fst->sign == -1 && snd->sign == -1) 
        neg = true;

    if (fst->size < snd->size)
    {
        if (neg)
            return 1;

        return -1;
    }
    else if (fst->size > snd->size)
    {
        if (neg)
            return -1;

        return 1;
    }
    else // fst->size == snd->size
    {
        for (size_t i = fst->size; i-- > 0;)
        {
            if (fst->digits[i] < snd->digits[i])
            {
                if (neg)
                    return 1;

                return -1;
            }
            else if (fst->digits[i] > snd->digits[i])
            {
                if (neg)
                    return -1;

                return 1;
            }
        }
    }

    return 0;
}

bool FOS_bignum_add_u32(FOS_Bignum *bn, uint32_t d)
{
    if (bn == NULL || bn->digits == NULL)
        return false;

    size_t i = 0;
    uint64_t carry = d;

    while (carry != 0 && i < bn->size)
    {
        uint64_t sum = (uint64_t)bn->digits[i] + carry;
        bn->digits[i] = (uint32_t)(sum % FOS_BIGNUM_BASE);
        carry = sum / FOS_BIGNUM_BASE;
        i++;
    }

    if (carry != 0)
    {
        // Need an extra digit
        if (!FOS_bignum_reserve(bn, bn->size + 1))
            return false;

        bn->digits[bn->size] = (uint32_t)carry;
        bn->size++;
    }

    return true;
}

bool FOS_bignum_sub_u32(FOS_Bignum *bn, uint32_t d)
{
    if (bn == NULL || bn->digits == NULL)
        return false;

    if (bn->size == 1 && bn->digits[0] < d)
        return false; // cannot subtract more than bn

    size_t i = 0;
    int64_t borrow = d;

    while (borrow != 0 && i < bn->size)
    {
        int64_t diff = (int64_t)bn->digits[i] - borrow;

        if (diff < 0)
        {
            diff += FOS_BIGNUM_BASE;
            borrow = 1;
        }
        else
        {
            borrow = 0;
        }

        bn->digits[i] = (uint32_t)diff;
        i++;
    }

    // Trim leading zeros
    while (bn->size > 1 && bn->digits[bn->size - 1] == 0)
        bn->size--;

    return true;
}

bool FOS_bignum_mul_u32(FOS_Bignum *bn, uint32_t d)
{
    if (bn == NULL || bn->digits == NULL)
        return false;

    if (d == 0)
    {
        bn->digits[0] = 0;
        bn->size = 1;
        bn->sign = +1;

        return true;
    }

    uint64_t carry = 0;

    for (size_t i = 0; i < bn->size; ++i)
    {
        uint64_t res = (uint64_t)bn->digits[i] * d + carry;

        bn->digits[i] = (uint32_t)(res % FOS_BIGNUM_BASE);
        carry = res / FOS_BIGNUM_BASE;
    }

    if (carry)
    {
        if (bn->size >= bn->capacity)
            if (!FOS_bignum_reserve(bn, bn->size + 1))
                return false;

        bn->digits[bn->size++] = (uint32_t)carry;
    }

    return true;
}

bool FOS_bignum_div_u32(FOS_Bignum *bn, uint32_t d, uint32_t *rem_out)
{
    if (bn == NULL || bn->digits == NULL || bn->size == 0 || d == 0)
        return false;

    uint64_t rem = 0;

    for (size_t i = bn->size; i-- > 0;)
    {
        uint64_t cur = bn->digits[i] + rem * FOS_BIGNUM_BASE;

        bn->digits[i] = (uint32_t)(cur / d);
        rem = cur % d;
    }

    FOS_bignum_trim(bn);

    if (rem_out)
        *rem_out = (uint32_t)rem;

    return true;
}

bool FOS_bignum_add(FOS_Bignum *res, const FOS_Bignum *a, const FOS_Bignum *b)
{
    if (res == NULL || a == NULL || b == NULL)
        return false;

    bool alias = (res == a) || (res == b);
    FOS_Bignum tmp;
    FOS_Bignum *out = res;

    if (alias)
    {
        if (!FOS_bignum_init(&tmp))
            return false;

        out = &tmp;
    }

    // Determine operation: addition or subtraction
    if (a->sign == b->sign)
    {
        // Same sign: magnitude addition
        size_t max = (a->size > b->size) ? a->size : b->size;

        if (out->capacity < max + 1)
        {
            if (!FOS_bignum_reserve(out, max + 1))
            {
                if (alias) 
                    FOS_bignum_free(&tmp);

                return false;
            }

            out->capacity = max + 1;
        }

        uint64_t carry = 0;

        for (size_t i = 0; i < max; ++i)
        {
            uint64_t sum = carry;

            if (i < a->size) 
                sum += a->digits[i];

            if (i < b->size) 
                sum += b->digits[i];

            out->digits[i] = (uint32_t)(sum % FOS_BIGNUM_BASE);
            carry = sum / FOS_BIGNUM_BASE;
        }

        if (carry)
        {
            out->digits[max] = (uint32_t)carry;
            out->size = max + 1;
        }
        else
        {
            out->size = max;
        }

        out->sign = a->sign; // keep the same sign

        if (out->size == 1 && out->digits[0] == 0)
            out->sign = +1;
    }
    else
    {
        // Different signs: subtract smaller magnitude from larger
        const FOS_Bignum *larger, *smaller;
        int cmp = FOS_bignum_cmp_abs(a, b); // compare magnitudes only
        if (cmp >= 0)
        {
            larger = a;
            smaller = b;
        }
        else
        {
            larger = b;
            smaller = a;
        }

        if (out->capacity < larger->size)
        {
            if (!FOS_bignum_reserve(out, larger->size))
            {
                if (alias) 
                    FOS_bignum_free(&tmp);

                return false;
            }

            out->capacity = larger->size;
        }

        int64_t borrow = 0;

        for (size_t i = 0; i < larger->size; ++i)
        {
            int64_t diff = (int64_t)larger->digits[i] - (i < smaller->size ? smaller->digits[i] : 0) - borrow;

            if (diff < 0)
            {
                diff += FOS_BIGNUM_BASE;
                borrow = 1;
            }
            else
            {
                borrow = 0;
            }

            out->digits[i] = (uint32_t)diff;
        }

        out->size = larger->size;
        out->sign = larger->sign;

        // Trim leading zeros
        FOS_bignum_trim(out);

        if (out->size == 1 && out->digits[0] == 0)
            out->sign = +1;
    }

    if (alias)
    {
        bool ok = FOS_bignum_copy(res, out);

        FOS_bignum_free(&tmp);

        return ok;
    }

    return true;
}

bool FOS_bignum_subtract(FOS_Bignum *res, const FOS_Bignum *a, const FOS_Bignum *b)
{
    if (res == NULL || a == NULL || b == NULL)
        return false;

    FOS_Bignum neg_b;

    if (!FOS_bignum_init(&neg_b))
        return false;

    if (!FOS_bignum_copy(&neg_b, b))
    {
        FOS_bignum_free(&neg_b);
        return false;
    }

    neg_b.sign = -neg_b.sign;

    bool ok = FOS_bignum_add(res, a, &neg_b);

    FOS_bignum_free(&neg_b);

    return ok;
}

bool FOS_bignum_multiply(FOS_Bignum *res, const FOS_Bignum *a, const FOS_Bignum *b)
{
    if (res == NULL || a == NULL || b == NULL)
        return false;

    bool alias = (res == a) || (res == b);

    FOS_Bignum tmp;
    FOS_Bignum *out = res;

    if (alias)
    {
        if (!FOS_bignum_init(&tmp))
            return false;

        out = &tmp;
    }

    const FOS_Bignum *fst = a;
    const FOS_Bignum *snd = b;

    FOS_Bignum temp;

    if (!FOS_bignum_init(&temp))
    {
        if (alias) FOS_bignum_free(&tmp);
        return false;
    }

    size_t max_digits = fst->size + snd->size;

    if (!FOS_bignum_reserve(&temp, max_digits))
    {
        FOS_bignum_free(&temp);
        if (alias) FOS_bignum_free(&tmp);
        return false;
    }

    memset(temp.digits, 0, max_digits * sizeof(uint32_t));
    temp.size = max_digits;
    temp.sign = +1;

    for (size_t j = 0; j < snd->size; ++j)
    {
        uint64_t carry = 0;

        for (size_t i = 0; i < fst->size; ++i)
        {
            uint64_t result = temp.digits[i + j]
                            + (uint64_t)fst->digits[i] * snd->digits[j]
                            + carry;

            temp.digits[i + j] = (uint32_t)(result % FOS_BIGNUM_BASE);
            carry = result / FOS_BIGNUM_BASE;
        }

        size_t k = fst->size + j;

        while (carry)
        {
            if (k >= temp.capacity)
            {
                FOS_bignum_free(&temp);
                if (alias) FOS_bignum_free(&tmp);
                return false;
            }

            uint64_t sum = (uint64_t)temp.digits[k] + carry;

            temp.digits[k] = (uint32_t)(sum % FOS_BIGNUM_BASE);
            carry = sum / FOS_BIGNUM_BASE;

            ++k;
        }
    }

    FOS_bignum_trim(&temp);  // sets correct size and handles zero

    if (out->capacity < temp.size)
    {
        if (!FOS_bignum_reserve(out, temp.size))
        {
            FOS_bignum_free(&temp);
            
            if (alias) 
                FOS_bignum_free(&tmp);

            return false;
        }
    }

    memcpy(out->digits, temp.digits, temp.size * sizeof(uint32_t));
    out->size = temp.size;

    if (a->sign == b->sign)
        out->sign = +1;
    else
        out->sign = -1;

    // optional: normalize zero sign if your trim doesn't guarantee it
    if (out->size == 1 && out->digits[0] == 0)
        out->sign = +1;

    FOS_bignum_free(&temp);

    if (alias)
    {
        bool ok = FOS_bignum_copy(res, out);
        FOS_bignum_free(&tmp);
        return ok;
    }

    return true;
}

bool FOS_bignum_divide(FOS_Bignum *quot, FOS_Bignum *rem, const FOS_Bignum *a, const FOS_Bignum *b)
{
    if (quot == NULL || rem == NULL || a == NULL || b == NULL)
        return false;

    bool alias =
        (quot == a) || (quot == b) ||
        (rem == a)  || (rem == b)  ||
        (quot == rem);

    FOS_Bignum tmp_q, tmp_r;
    FOS_Bignum *out_q = quot;
    FOS_Bignum *out_r = rem;

    if (alias)
    {
        if (!FOS_bignum_init(&tmp_q))
            return false;

        if (!FOS_bignum_init(&tmp_r))
        {
            FOS_bignum_free(&tmp_q);
            return false;
        }

        out_q = &tmp_q;
        out_r = &tmp_r;
    }

    if (b->digits == NULL || b->size == 0)
    {
        if (alias)
            goto cleanup;

        return false;
    }

    if (b->size == 1 && b->digits[0] == 0)
    {
        if (alias)
            goto cleanup;

        return false;
    }

    int cmp_res = FOS_bignum_cmp(a, b);

    if (cmp_res < 0)
    {
        if (!FOS_bignum_reserve(out_q, 1))
        {
            if (alias)
                goto cleanup;

            return false;
        }

        out_q->digits[0] = 0;
        out_q->size = 1;
        out_q->sign = +1;

        if (!FOS_bignum_copy(out_r, a))
        {
            if (alias)
                goto cleanup;

            return false;
        }

        if (alias)
        {
            bool success =
                FOS_bignum_copy(quot, out_q) &&
                FOS_bignum_copy(rem, out_r);

            FOS_bignum_free(&tmp_q);
            FOS_bignum_free(&tmp_r);

            return success;
        }

        return true;
    }

    if (cmp_res == 0)
    {
        if (!FOS_bignum_reserve(out_q, 1))
        {
            if (alias)
                goto cleanup;

            return false;
        }

        out_q->digits[0] = 1;
        out_q->size = 1;
        out_q->sign = +1;

        if (!FOS_bignum_reserve(out_r, 1))
        {
            if (alias)
                goto cleanup;

            return false;
        }

        out_r->digits[0] = 0;
        out_r->size = 1;
        out_r->sign = a->sign;

        if (alias)
        {
            bool success =
                FOS_bignum_copy(quot, out_q) &&
                FOS_bignum_copy(rem, out_r);

            FOS_bignum_free(&tmp_q);
            FOS_bignum_free(&tmp_r);

            return success;
        }

        return true;
    }

    FOS_Bignum u;

    if (!FOS_bignum_init(&u))
    {
        if (alias)
            goto cleanup;

        return false;
    }

    if (!FOS_bignum_copy(&u, a))
    {
        FOS_bignum_free(&u);

        if (alias)
            goto cleanup;

        return false;
    }

    FOS_Bignum v;

    if (!FOS_bignum_init(&v))
    {
        FOS_bignum_free(&u);

        if (alias)
            goto cleanup;

        return false;
    }

    if (!FOS_bignum_copy(&v, b))
    {
        FOS_bignum_free(&u);
        FOS_bignum_free(&v);

        if (alias)
            goto cleanup;

        return false;
    }

    if (u.capacity <= u.size)
        if (!FOS_bignum_reserve(&u, u.size + 1))
        {
            FOS_bignum_free(&u);
            FOS_bignum_free(&v);

            if (alias)
                goto cleanup;

            return false;
        }

    uint32_t v_msd = v.digits[v.size - 1];

    uint32_t d = FOS_BIGNUM_BASE / (v_msd + 1);

    if (d > 1)
    {
        if (!FOS_bignum_mul_u32(&u, d)) 
        { 
            FOS_bignum_free(&u); 
            FOS_bignum_free(&v);

            if (alias)
                goto cleanup;

            return false; 
        }

        if (!FOS_bignum_mul_u32(&v, d)) 
        { 
            FOS_bignum_free(&u); 
            FOS_bignum_free(&v);

            if (alias)
                goto cleanup;
 
            return false; 
        }
    }

    u.digits[u.size++] = 0;

    size_t n = v.size;
    size_t m = u.size - n - 1;

    if (!FOS_bignum_reserve(out_q, m + 1))
    {
        FOS_bignum_free(&u);
        FOS_bignum_free(&v);

        if (alias)
            goto cleanup;

        return false;
    }

    out_q->size = m + 1;
    out_q->sign = a->sign * b->sign;

    for (size_t j = m + 1; j-- > 0;)
    {
        uint64_t u2 = u.digits[j + n];
        uint64_t u1 = u.digits[j + n - 1];
        uint64_t u0 = (n >= 2) ? u.digits[j + n - 2] : 0;

        uint64_t v1 = v.digits[n - 1];
        uint64_t v0 = (n >= 2) ? v.digits[n - 2] : 0;

        uint64_t num = u2 * FOS_BIGNUM_BASE + u1;

        uint64_t qhat = num / v1;
        uint64_t rhat = num % v1;

        if (qhat >= FOS_BIGNUM_BASE)
            qhat = FOS_BIGNUM_BASE - 1;

        while (n >= 2 && qhat * v0 > FOS_BIGNUM_BASE * rhat + u0)
        {
            qhat--;
            rhat += v1;

            if (rhat >= FOS_BIGNUM_BASE)
                break;
        }

        int64_t borrow = 0;

        for (size_t i = 0; i < n; ++i)
        {
            uint64_t p = qhat * v.digits[i];

            int64_t sub = (int64_t)u.digits[j + i]
                        - (int64_t)(p % FOS_BIGNUM_BASE)
                        - borrow;

            borrow = (int64_t)(p / FOS_BIGNUM_BASE);

            if (sub < 0)
            {
                sub += FOS_BIGNUM_BASE;
                borrow += 1;
            }

            u.digits[j + i] = (uint32_t)sub;
        }

        int64_t sub = (int64_t)u.digits[j + n] - borrow;
        bool need_correction = false;

        if (sub < 0)
        {
            u.digits[j + n] = (uint32_t)(sub + FOS_BIGNUM_BASE);
            need_correction = true;
        }
        else
        {
            u.digits[j + n] = (uint32_t)sub;
        }

        if (need_correction)
        {
            qhat--;

            uint64_t carry = 0;

            for (size_t i = 0; i < n; ++i)
            {
                uint64_t sum = (uint64_t)u.digits[j + i]
                             + v.digits[i]
                             + carry;

                if (sum >= FOS_BIGNUM_BASE)
                {
                    u.digits[j + i] = (uint32_t)(sum - FOS_BIGNUM_BASE);
                    carry = 1;
                }
                else
                {
                    u.digits[j + i] = (uint32_t)sum;
                    carry = 0;
                }
            }

            u.digits[j + n] += (uint32_t)carry;
        }

        out_q->digits[j] = (uint32_t)qhat;
    }

    if (!FOS_bignum_copy(out_r, &u))
    {
        FOS_bignum_free(&u);
        FOS_bignum_free(&v);

        if (alias)
        {
            FOS_bignum_free(&tmp_q);
            FOS_bignum_free(&tmp_r);
        }

        return false;
    }

    if (d > 1)
    {
        uint32_t tmp;

        if (!FOS_bignum_div_u32(out_r, d, &tmp))
        {
            FOS_bignum_free(&u);
            FOS_bignum_free(&v);
            
            if (alias)
            {
                FOS_bignum_free(&tmp_q);
                FOS_bignum_free(&tmp_r);
            }
            return false;
        }
    }

    out_r->sign = a->sign;

    FOS_bignum_trim(out_q);
    FOS_bignum_trim(out_r);

    FOS_bignum_free(&u);
    FOS_bignum_free(&v);

    if (alias)
    {
        bool success =
            FOS_bignum_copy(quot, out_q) &&
            FOS_bignum_copy(rem, out_r);

        FOS_bignum_free(&tmp_q);
        FOS_bignum_free(&tmp_r);

        return success;
    }

    return true;

cleanup:
    FOS_bignum_free(&tmp_q);
    FOS_bignum_free(&tmp_r);

    return false;
}

bool FOS_bignum_from_cstr(FOS_Bignum *bn, const char *num)
{
    if (bn == NULL || num == NULL)
        return false;

    size_t len = strlen(num);

    if (fos_bignum_max_digits != 0 &&
        len > fos_bignum_max_digits * 9)
        return false;

    if (!FOS_bignum_reserve(bn, 8))
        return false;

    bn->size = 1;
    bn->digits[0] = 0;
    bn->sign = +1;

    int sign = +1;
    size_t start = 0;

    if (num[0] == '-') 
    { 
        sign = -1; 
        start = 1; 
    }
    else if (num[0] == '+') 
        start = 1; 

    for (size_t i = start; i < len; ++i)
    {
        if (!isdigit((unsigned char)num[i]))
            return false;

        uint32_t digit = (uint32_t)(num[i] - '0');
        
        if (!FOS_bignum_mul_u32(bn, 10))
            return false;

        if (!FOS_bignum_add_u32(bn, digit))
            return false;
    }

    bn->sign = sign;
    FOS_bignum_trim(bn);

    return true;
}
bool FOS_bignum_to_cstr(const FOS_Bignum *bn, char *buf, size_t buf_size)
{
    if (bn == NULL || buf == NULL || buf_size == 0)
        return false;

    if (bn->size == 0)
    {
        if (buf_size < 2)
            return false;

        buf[0] = '0';
        buf[1] = '\0';
        return true;
    }

    size_t pos = 0;

    // Handle sign
    if (bn->sign < 0)
    {
        if (pos + 1 >= buf_size)
            return false;

        buf[pos++] = '-';
    }

    // Print most significant digit
    uint32_t msd = bn->digits[bn->size - 1];

    int written = snprintf(buf + pos, buf_size - pos, "%u", msd);
    if (written < 0 || (size_t)written >= buf_size - pos)
        return false;

    pos += (size_t)written;

    // Print remaining digits with zero padding (9 digits each)
    for (size_t i = bn->size - 1; i-- > 0;)
    {
        if (pos + 9 >= buf_size)
            return false;

        written = snprintf(buf + pos, buf_size - pos, "%09u", bn->digits[i]);
        if (written != 9)
            return false;

        pos += 9;
    }

    // Null-terminate
    if (pos >= buf_size)
        return false;

    buf[pos] = '\0';
    return true;
}