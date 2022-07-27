#include "mirrored-allocators-internal.h"


#undef  utarray_oom
#define utarray_oom() { \
	MAM_RAISE_ERR_GOTO(err, MAM_EOM, err_mem); \
}

mam_error_t
mam_allocator_create(mam_allocator_t *allocator) {
	MAM_CHECK_PTR(allocator);

	mam_error_t err = MAM_SUCCESS;
	struct _mam_allocator_s *alloc = 
		calloc(1, sizeof(struct _mam_allocator_s));
	MAM_REFUTE(!alloc, MAM_EOM);
	utarray_new(alloc->buffers, &_mam_buff_desc_icd);
	*allocator = alloc;
	return MAM_SUCCESS;
err_mem:
	free(alloc);
	return err;
}

mam_error_t
mam_allocator_alloc(
		mam_allocator_t   allocator,
		void            **addr,
		void            **mirror_addr,
		mam_buff_desc_t **desc) {
	MAM_CHECK_PTR(allocator);
	MAM_CHECK_PTR(addr);
	MAM_CHECK_PTR(mirror_addr);
	MAM_CHECK_PTR(desc);
	return MAM_SUCCESS;
}
