#ifndef MIRRORED_ALLOCATORS_INTERNAL_H
#define MIRRORED_ALLOCATORS_INTERNAL_H

#include <mirrored-allocators.h>
#include "utarray.h"

/**
 * The statement x is likely evaluating to true.
 */
#define MAM_LIKELY(x)      __builtin_expect(!!(x), 1)
/**
 * The statement x is likely evaluating to false.
 */
#define MAM_UNLIKELY(x)    __builtin_expect(!!(x), 0)

#define MAM_REFUTE(cond, error) do { \
	if (MAM_UNLIKELY(cond))      \
		return error;        \
} while (0)

#define MAM_REFUTE_ERR_GOTO(err, cond, error, label)  do { \
	if (MAM_UNLIKELY(cond)) {                          \
		err = error;                               \
		goto label;                                \
	}                                                  \
} while (0)

#define MAM_CHECK_PTR(p) MAM_REFUTE(!(p), MAM_EINVAL)
#define MAM_CHECK_ARY(c, a) MAM_REFUTE((c > 0) && !(a), MAM_EINVAL)

#define MAM_RAISE_ERR_GOTO(err, error, label) do { \
	err = error;                               \
	goto label;                                \
} while (0)

#define MAM_VALIDATE_ERR(err, cmd) do {      \
	err = (cmd);                         \
	if (MAM_UNLIKELY(err < MAM_SUCCESS)) \
		return err;                  \
} while (0)

#define MAM_VALIDATE(cmd) do { \
	mam_error_t _err; \
	MAM_VALIDATE_ERR(_err, cmd); \
} while(0)

#define MAM_ALLOC_SIZE_BITS 12
#define MAM_ALLOC_SIZE_DEFAULT (1 << MAM_ALLOC_SIZE_BITS)
#define MAM_SIZE_CUTOFF 0x10

struct _mam_allocator_s {
	size_t                total_size;
	mam_platform_alloc_t  alloc1;
	mam_platform_alloc_t  alloc2;
	UT_array             *buffers;
};

static const UT_icd _mam_buff_desc_icd = {
	sizeof(mam_buff_desc_t *),
	NULL,
	NULL,
	NULL,
};

#endif //MIRRORED_ALLOCATORS_INTERNAL_H
