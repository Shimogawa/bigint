#ifndef BIGINT_H
#define BIGINT_H

#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define BIGINT_FLAG_NEG 0x01

typedef uint32_t bint_blk_type;
#define BINT_BLK_SZ sizeof(bint_blk_type)
#define BINT_BLK_BIT_SZ (BINT_BLK_SZ << 3)
#define BINT_BLK_MAX UINT32_MAX

static const bint_blk_type BINT_BLK_HIGHEST_BIT =
    1u << (sizeof(bint_blk_type) * 8 - 1);

// #define MAKE_UINT64(uHi, uLo) ((uHi) << 32 | (uLo))

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define BINT_REALLOC(bi, sz)                                   \
    bint_blk_type* tmp =                                       \
        (bint_blk_type*)realloc(bi->data, BINT_BLK_SZ * (sz)); \
    if (!tmp) return 1;                                        \
    bi->data = tmp;                                            \
    bi->n = (sz);

typedef struct {
    size_t n;            /* number of blocks */
    bint_blk_type* data; /* data in blocks */
    uint8_t flags;       /* flags */
} bigint;

/// Makes an empty bigint (with NULL data)
bigint* BINT_make();
/// Makes a bigint initialized to 0.
bigint* BINT_zero();
bigint* BINT_makei(int32_t i);
bigint* BINT_makeui(uint32_t i);
bigint* BINT_makel(int64_t l);
bigint* BINT_makeul(uint64_t l);
bigint* BINT_makep(void* p, size_t size);

/// Make a copy of bi.
bigint* BINT_cp(const bigint* bi);
/// Set the value of bi to i.
void BINT_setui(bigint* bi, uint32_t i);

bigint* BINT_atoi(const char* str);
char* BINT_itoa(const bigint* bi);

/// Shift left n bits.
/// @returns the status. 0 for success, 1 for failure.
int BINT_shl(bigint* bi, size_t nbit);
/// Shift left 1 bit.
int BINT_shl1(bigint* bi);
int BINT_shr1(bigint* bi);
/// Frees the bigint.
void BINT_free(bigint* bi);
/// Checks if the bigint is negative or not.
bool BINT_isneg(const bigint* bi);
/// Checks if the bigint is 0.
bool BINT_iszero(const bigint* bi);
/// Negates a bigint.
void BINT_neg(bigint* bi);

int BINT_cmp(const bigint* b1, const bigint* b2);
/// Removes leading zero blocks.
int BINT_rlz(bigint* b);

/// d / n -> res ... n
/// Divide n by d, return the result, and put remainder in n.
bigint* BINT_divmod(bigint* n, const bigint* div);
/**
 * @brief Multiply b by an immediate 32-bit unsigned integer value imm.
 *
 * @param b bigint as receiver
 * @param imm the immediate
 * @return the return code. 0 for success, 1 for failure.
 */
int BINT_multo_imm(bigint* b, uint32_t imm);
int BINT_mul(const bigint* l, const bigint* r, bigint* res);

/// Add an immediate value imm to a bigint b.
int BINT_addto_imm(bigint* b, uint32_t imm);
int BINT_add(const bigint* l, const bigint* r, bigint* res);
/// Subtract b from a. Assumed a >= b.
/// a -= b.
int BINT_sub_from(bigint* a, const bigint* b);

int BINT_set_bit_at(bigint* bi, size_t idx, bool set);

void _bint_init_with_size(bigint* bi, size_t n, void* val);

#endif