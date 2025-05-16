

#include <stdint.h>

#ifdef _MSC_VER
#define ALIGN(ll, a) \
    __declspec(align(a)) ll
#else
#define ALIGN(line, align) \
    line __attribute__((aligned(align)))
#endif

#ifndef static_assert
#define CHECK_OFFSET(type, field, name) \
    struct check_##type##_##field { int x[(name == offsetof(type, field)) ? 1 : -1]; }
#define CHECK_SIZE(type, size) \
    struct check_##type##_size { int x[(size == sizeof(type)) ? 1 : -1]; }
#else
#define CHECK_OFFSET(type, field, name) \
    static_assert(name == offsetof(type, field), #field)
#define CHECK_SIZE(type, size) \
    static_assert(size == sizeof(type), #type)
#endif

#ifdef _MSC_VER
#define PACKED(...) __pragma(pack(push, 1)) __VA_ARGS__ __pragma(pack(pop))
#else
#define PACKED(...) __VA_ARGS__ __attribute__((__packed__))
#endif

#define INVALID_MV 0x80008000

enum BlockSize {
    BS_128x128,
    BS_128x64,
    BS_64x128,
    BS_64x64,
    BS_64x32,
    BS_64x16,
    BS_32x64,
    BS_32x32,
    BS_32x16,
    BS_32x8,
    BS_16x64,
    BS_16x32,
    BS_16x16,
    BS_16x8,
    BS_16x4,
    BS_8x32,
    BS_8x16,
    BS_8x8,
    BS_8x4,
    BS_4x16,
    BS_4x8,
    BS_4x4,
    N_BS_SIZES,
};

typedef union mv {
    struct {
        int16_t y, x;
    };
    uint32_t n;
} mv;

PACKED(typedef struct refmvs_temporal_block {
    mv mv;
    int8_t ref;
}) refmvs_temporal_block;
CHECK_SIZE(refmvs_temporal_block, 5);

PACKED(typedef union refmvs_refpair {
    int8_t ref[2]; // [0] = 0: intra=1, [1] = -1: comp=0
    uint16_t pair;
}) ALIGN(refmvs_refpair, 2);
CHECK_SIZE(refmvs_refpair, 2);

typedef union refmvs_mvpair {
    mv mv[2];
    uint64_t n;
} refmvs_mvpair;
CHECK_SIZE(refmvs_mvpair, 8);

PACKED(typedef struct refmvs_block {
    refmvs_mvpair mv;
    refmvs_refpair ref;
    uint8_t bs, mf; // 1 = globalmv+affine, 2 = newmv
}) ALIGN(refmvs_block, 4);
CHECK_SIZE(refmvs_block, 12);

typedef struct refmvs_candidate {
    refmvs_mvpair mv;
    int weight;
} refmvs_candidate;


static void add_spatial_candidate(refmvs_candidate *const mvstack, int *const cnt,
                                  const int weight, const refmvs_block *const b,
                                  const union refmvs_refpair ref, const mv gmv[2],
                                  int *const have_newmv_match,
                                  int *const have_refmv_match)
{
    if (b->mv.mv[0].n == INVALID_MV) return; // intra block, no intrabc

    if (ref.ref[1] == -1) {
        for (int n = 0; n < 2; n++) {
            if (b->ref.ref[n] == ref.ref[0]) {
                const mv cand_mv = ((b->mf & 1) && gmv[0].n != INVALID_MV) ?
                                   gmv[0] : b->mv.mv[n];

                *have_refmv_match = 1;
                *have_newmv_match |= b->mf >> 1;

                const int last = *cnt;
                for (int m = 0; m < last; m++)
                    if (mvstack[m].mv.mv[0].n == cand_mv.n) {
                        mvstack[m].weight += weight;
                        return;
                    }

                if (last < 8) {
                    mvstack[last].mv.mv[0] = cand_mv;
                    mvstack[last].weight = weight;
                    *cnt = last + 1;
                }
                return;
            }
        }
    } else if (b->ref.pair == ref.pair) {
        const refmvs_mvpair cand_mv = { .mv = {
            [0] = ((b->mf & 1) && gmv[0].n != INVALID_MV) ? gmv[0] : b->mv.mv[0],
            [1] = ((b->mf & 1) && gmv[1].n != INVALID_MV) ? gmv[1] : b->mv.mv[1],
        }};

        *have_refmv_match = 1;
        *have_newmv_match |= b->mf >> 1;

        const int last = *cnt;
        for (int n = 0; n < last; n++)
            if (mvstack[n].mv.n == cand_mv.n) {
                mvstack[n].weight += weight;
                return;
            }

        if (last < 8) {
            mvstack[last].mv = cand_mv;
            mvstack[last].weight = weight;
            *cnt = last + 1;
        }
    }
}

void sample(void) {
    refmvs_candidate mvstack[8] = {
        {
            .mv = {
                .mv = {{0, 0}, {0, 0}}
            },
            .weight = 0
        },
        {
            .mv = {
                .mv = {{0, 0}, {0, 0}}
            },
            .weight = 0
        },
        {
            .mv = {
                .mv = {{0, 0}, {0, 0}}
            },
            .weight = 0
        },
        {
            .mv = {
                .mv = {{0, 0}, {0, 0}}
            },
            .weight = 0
        },
        {
            .mv = {
                .mv = {{0, 0}, {0, 0}}
            },
            .weight = 0
        },
        {
            .mv = {
                .mv = {{0, 0}, {0, 0}}
            },
            .weight = 0
        },
        {
            .mv = {
                .mv = {{0, 0}, {0, 0}}
            },
            .weight = 0
        },
        {
            .mv = {
                .mv = {{0, 0}, {0, 0}}
            },
            .weight = 0
        }
    };
    
    int cnt = 0;
    int weight = 192;
    
    refmvs_block b = {
        .mv = {
            .mv = {{0, 0}, {0, 0}}
        },
        .ref = {
            .ref = {1, -1}
        },
        .bs = BS_128x128,
        .mf = 0
    };
    
    refmvs_refpair ref_pair = {
        .ref = {1, -1}
    };
    
    mv gmv[2] = {
        {.y = -32768, .x = -32768},
        {.y = 0, .x = 0}
    };
    
    int have_newmv_match = 0;
    int have_refmv_match = 0;

    add_spatial_candidate(
        mvstack,
        &cnt,
        weight,
        &b,
        ref_pair,
        gmv,
        &have_newmv_match,
        &have_refmv_match
    );

    // Equivalent to black_box in Rust (prevents compiler optimization)
    // In C, this might be implemented with volatile or compiler-specific directives
    asm volatile("" : : "r"(mvstack) : "memory");
}

int main(void) {
    for (long long i = 0; i < 1000000000LL; i++) {
        sample();
    }
    return 0;
}
