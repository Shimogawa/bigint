#include "bigint.h"

#include "strbuf.h"

bigint* BINT_make() {
    bigint* bi = (bigint*)malloc(sizeof(bigint));
    bi->n = 0;
    bi->data = NULL;
    bi->flags = 0;
    return bi;
}

bigint* BINT_zero() {
    bigint* bi = (bigint*)malloc(sizeof(bigint));
    _bint_init_with_size(bi, 1, NULL);
    bi->data[0] = 0;
    return bi;
}

bigint* BINT_makei(int32_t i) {
    bigint* bi = (bigint*)malloc(sizeof(bigint));
    if (i < 0) {
        bi->flags = BIGINT_FLAG_NEG;
        i = -i;
    }
    _bint_init_with_size(bi, 1, &i);
    return bi;
}

bigint* BINT_makeui(uint32_t i) {
    bigint* bi = (bigint*)malloc(sizeof(bigint));
    _bint_init_with_size(bi, 1, &i);
    return bi;
}

bigint* BINT_makel(int64_t l) {
    bool neg = l < 0;
    bigint* res = BINT_makeul((uint64_t)(neg ? -l : l));
    res->flags |= BIGINT_FLAG_NEG;
    return res;
}

bigint* BINT_makeul(uint64_t l) {
    bigint* bi = (bigint*)malloc(sizeof(bigint));
    _bint_init_with_size(bi, 2, &l);
    return bi;
}

bigint* BINT_makep(void* p, size_t size) {
    bigint* bi = (bigint*)malloc(sizeof(bigint));
    _bint_init_with_size(bi, size, p);
    return bi;
}

bigint* BINT_cp(const bigint* bi) {
    bigint* cp = BINT_make();
    cp->n = bi->n;
    cp->flags = bi->flags;
    cp->data = (bint_blk_type*)malloc(BINT_BLK_SZ * cp->n);
    if (!cp->data) {
        free(cp);
        return NULL;
    }
    memcpy(cp->data, bi->data, BINT_BLK_SZ * cp->n);
    return cp;
}

int BINT_shl(bigint* bi, size_t nbit) {
    int leading_zeroes = __builtin_clz(bi->data[bi->n - 1]);
    size_t add_zero_blks = nbit / BINT_BLK_BIT_SZ;
    nbit = nbit % BINT_BLK_BIT_SZ;
    bool new_lead_blk = nbit > leading_zeroes;
    size_t add_blk_cnt = add_zero_blks + new_lead_blk;
    bint_blk_type* buf =
        (bint_blk_type*)malloc((bi->n + add_blk_cnt) * BINT_BLK_SZ);
    bint_blk_type carry = 0;
    bint_blk_type cur;
    int j = 0;
    for (; j < add_zero_blks; j++) {
        buf[j] = 0u;
    }
    for (int i = 0; i < bi->n; i++, j++) {
        cur = bi->data[i] + carry;
        carry = cur >> (BINT_BLK_BIT_SZ - nbit);
        buf[j] = cur << nbit;
    }
    if (new_lead_blk) {
        if (!carry) {
            free(buf);
            return 1;
        }
        buf[j] = carry;
    }
    free(bi->data);
    bi->data = buf;
    return 0;
}

int BINT_shl1(bigint* bi) {
    bint_blk_type carry = 0;
    bint_blk_type next_carry;
    for (size_t i = 0; i < bi->n; i++) {
        next_carry = (bi->data[i] & BINT_BLK_HIGHEST_BIT) ? 1 : 0;
        bi->data[i] = (bi->data[i] << 1) | carry;
        carry = next_carry;
    }
    if (carry) {
        BINT_REALLOC(bi, bi->n + 1);
        bi->data[bi->n - 1] = 1u;
    }
    return 0;
}

int BINT_shr1(bigint* bi) {
    bint_blk_type carry = 0;
    bint_blk_type next_carry;
    for (size_t i = bi->n - 1; i + 1; i--) {
        next_carry = (bi->data[i] & 1) ? BINT_BLK_HIGHEST_BIT : 0;
        bi->data[i] = (bi->data[i] >> 1) | carry;
        carry = next_carry;
    }
    return BINT_rlz(bi);
}

inline void BINT_free(bigint* bi) {
    free(bi->data);
    free(bi);
}

inline bool BINT_isneg(const bigint* bi) { return bi->flags & BIGINT_FLAG_NEG; }

bool BINT_iszero(const bigint* bi) {
    for (size_t i = 0; i < bi->n; i++) {
        if (bi->data[i] != 0) {
            return false;
        }
    }
    return true;
}

inline void BINT_neg(bigint* bi) { bi->flags ^= BIGINT_FLAG_NEG; }

int BINT_cmp(const bigint* b1, const bigint* b2) {
    size_t max_size = MAX(b1->n, b2->n);
    for (size_t i = max_size - 1; i + 1; i--) {
        bint_blk_type wa = i < b1->n ? b1->data[i] : 0;
        bint_blk_type wb = i < b2->n ? b2->data[i] : 0;
        if (wa != wb) {
            return wa > wb ? 1 : -1;
        }
    }
    return 0;
}

int BINT_rlz(bigint* b) {
    size_t size = b->n;
    if (size == 0 || b->data[size - 1] != 0) return 0;
    for (size_t i = size - 2; i + 1; i--) {
        if (b->data[i] != 0) {
            BINT_REALLOC(b, i + 1);
            return 0;
        }
    }
    bint_blk_type* tmp = (bint_blk_type*)malloc(BINT_BLK_SZ * b->n);
    if (!tmp) return 1;
    free(b->data);
    b->data = tmp;
    b->n = 1;
    b->data[0] = 0;
    return 0;
}

bigint* BINT_divmod(bigint* n, const bigint* div) {
    if (BINT_iszero(div)) return NULL;
    bigint* d = BINT_cp(div);
    size_t shift = 0;
    while (BINT_cmp(n, d) > 0) {
        BINT_shl1(d);
        shift++;
    }
    bigint* result = BINT_make();
    while (1) {
        if (BINT_cmp(n, d) >= 0) {
            BINT_set_bit_at(result, shift, true);
            BINT_sub_from(n, d);
        }
        BINT_shr1(d);
        if (shift == 0) break;
        shift--;
    }
    BINT_rlz(n);
    BINT_rlz(result);
    BINT_free(d);
    return result;
}

int BINT_mul(const bigint* l, const bigint* r, bigint* res) {}

int BINT_add(const bigint* l, const bigint* r, bigint* res) {
    bint_blk_type carry = 0;
    uint64_t tmp;
    const bigint* big = (l->n >= r->n) ? l : r;
    const bigint* small = (big == l) ? r : l;
    bint_blk_type* buf = (bint_blk_type*)malloc(BINT_BLK_SZ * (big->n + 1));
    // TODO: negative
    if (!buf) {
        return 1;
    }
    size_t i = 0;
    for (; i < small->n; i++) {
        tmp = (uint64_t)big->data[i] + small->data[i] + carry;
        carry = tmp >> BINT_BLK_BIT_SZ;
        buf[i] = (bint_blk_type)tmp;
    }
    for (; i < big->n; i++) {
        tmp = (uint64_t)big->data[i] + carry;
        if (tmp > BINT_BLK_MAX) {
            carry = tmp >> BINT_BLK_BIT_SZ;
        }
        buf[i] = (bint_blk_type)tmp;
    }
    res->n = big->n;
    if (carry > 0) {
        buf[i] = carry;
        res->n++;
    }
    res->data = buf;
    return 0;
}

int BINT_sub_from(bigint* a, const bigint* b) {
    bint_blk_type borrow = 0;
    bint_blk_type tmp;
    for (size_t i = 0; i < b->n; i++) {
        if (b->data[i] || borrow) {
            tmp = a->data[i] - b->data[i] - borrow;
            borrow = tmp >= a->data[i];
            a->data[i] = tmp;
        }
    }
    for (size_t i = b->n; borrow != 0; i++) {
        borrow = a->data[i] == 0;
        a->data[i]--;
    }
    return 0;
}

int BINT_set_bit_at(bigint* bi, size_t idx, bool set) {
    size_t blk_idx = idx / BINT_BLK_BIT_SZ;
    size_t blk_off = idx % BINT_BLK_BIT_SZ;
    if (bi->n <= blk_idx) {
        size_t orig_size = bi->n;
        BINT_REALLOC(bi, blk_idx + 1);

        for (size_t i = orig_size; i < bi->n; i++) {
            bi->data[i] = 0;
        }
    }
    if (set) {
        bi->data[blk_idx] |= 1u << blk_off;
    } else {
        bi->data[blk_idx] &= ~(1u << blk_off);
    }
    return 0;
}

char* BINT_itoa(const bigint* bi) {
    strbuf* sb = STRBUF_new();
    bigint* ten = BINT_makeui(10u);

    bigint* bicp = BINT_cp(bi);
    bigint* next;
    char digit;
    while (1) {
        next = BINT_divmod(bicp, ten);
        digit = ((bicp->n > 0) ? bicp->data[0] : 0) + '0';
        STRBUF_pushback(sb, digit);
        BINT_free(bicp);
        bicp = next;
        if (BINT_iszero(bicp)) break;
    }
    BINT_free(bicp);
    BINT_free(ten);
    if (BINT_isneg(bi)) {
        STRBUF_pushback(sb, '-');
    }
    STRBUF_reverse(sb);

    char* res = STRBUF_tocstr(sb);
    STRBUF_free(sb);
    return res;
}

inline void _bint_init_with_size(bigint* bi, size_t n, void* val) {
    bi->n = n;
    bi->flags = 0;
    bi->data = (bint_blk_type*)malloc(n * BINT_BLK_SZ);
    if (val != NULL) {
        memcpy(bi->data, val, n * BINT_BLK_SZ);
    }
}
