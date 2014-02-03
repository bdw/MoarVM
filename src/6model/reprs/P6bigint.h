#include "tommath.h"

/* Another way to interpret the MVMP6bigintBody */
struct MVMP6littlebigintBody {
    MVMint32 data;
    MVMint32 flag;
};

/* Representation used by P6 Ints. */
struct MVMP6bigintBody {
#if !defined(_M_X64) && !defined(__amd64__)
    // if mp_int ends up being only 32 bits wide, we need some extra padding
    // otherwise casting MVMP6bigintBody* to MVMP6littlebigintBody* will fail
    // miserably.
    MVMint32 pad;
#endif
    /* Big integer storage slot. */
    mp_int *i;
};
struct MVMP6bigint {
    MVMObject common;
    MVMP6bigintBody body;
};

/* Function for REPR setup. */
const MVMREPROps * MVMP6bigint_initialize(MVMThreadContext *tc);
