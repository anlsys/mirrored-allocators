#ifndef MIRRORED_ALLOCATORS_INTERNAL_H
#define MIRRORED_ALLOCATORS_INTERNAL_H

#include <mam.h>
#include "utarray.h"

#define MAM_ALLOC_SIZE_BITS 12
#define MAM_ALLOC_SIZE_DEFAULT (1 << MAM_ALLOC_SIZE_BITS)
#define MAM_SIZE_CUTOFF 0x10

struct _mam_allocator_s {
	size_t                total_size;
	mam_platform_alloc_t  alloc1;
	mam_platform_alloc_t  alloc2;
	mam_buff_desc_t      *current_buffer;
	UT_array             *buffers;
};

static const UT_icd _mam_buff_desc_icd = {
	sizeof(mam_buff_desc_t *),
	NULL,
	NULL,
	NULL,
};

#endif //MIRRORED_ALLOCATORS_INTERNAL_H
