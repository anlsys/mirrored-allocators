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

#define MAM_CHECK_PTR(p) MAM_REFUTE(!(p), MAM_EINVAL)

#define MAM_RAISE_ERR_GOTO(err, error, label) do { \
	err = error;                               \
	goto label;                                \
} while (0)

#define MAM_ALLOC_SIZE_DEFAULT 4096

struct _mam_allocator_s {
	UT_array *buffers;
};

static const UT_icd _mam_buff_desc_icd = {
	sizeof(mam_buff_desc_t *),
	NULL,
	NULL,
	NULL,
};

#endif //MIRRORED_ALLOCATORS_INTERNAL_H
