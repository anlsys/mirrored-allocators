#include "mam-internal.h"
#include "mirrored-allocators-internal.h"
#include <stdint.h>
#include <stdio.h>

#undef  utarray_oom
#define utarray_oom() { \
	MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_mem); \
}

mam_error_t
mam_allocator_create(
		mam_allocator_t      *allocator,
		mam_platform_alloc_t *alloc1,
		mam_platform_alloc_t *alloc2) {
	MAM_CHECK_PTR(allocator);
	MAM_CHECK_PTR(alloc1);
	MAM_CHECK_PTR(alloc2);
	MAM_CHECK_PTR(alloc1->alloc);
	MAM_CHECK_PTR(alloc1->free);
	MAM_CHECK_PTR(alloc2->alloc);
	MAM_CHECK_PTR(alloc2->free);

	mam_error_t err = MAM_SUCCESS;
	struct _mam_allocator_s *alloc =
		calloc(1, sizeof(struct _mam_allocator_s));
	MAM_REFUTE(!alloc, MAM_ENOMEM);
	utarray_new(alloc->buffers, &_mam_buff_desc_icd);
	alloc->alloc1 = *alloc1;
	alloc->alloc2 = *alloc2;
	*allocator = alloc;
	return MAM_SUCCESS;
err_mem:
	free(alloc);
	return err;
}

#undef  utarray_oom

static inline void
_mam_free_buffer(
		mam_allocator_t  allocator,
		mam_buff_desc_t *buffer) {
	allocator->alloc1.free(buffer->addr);
	allocator->alloc2.free(buffer->m_addr);
	free(buffer);
}

mam_error_t
mam_allocator_destroy(
		mam_allocator_t allocator) {
	MAM_CHECK_PTR(allocator);
	UT_array *array = allocator->buffers;
	mam_buff_desc_t **p_buffer = NULL;
	while ( (p_buffer =  (mam_buff_desc_t **)utarray_next(array, p_buffer)) )
		_mam_free_buffer(allocator, *p_buffer);
	if (allocator->current_buffer)
		_mam_free_buffer(allocator, allocator->current_buffer);
	if (array)
		utarray_free(array);
	free(allocator);
	return MAM_SUCCESS;
}


static inline mam_error_t
_mam_allocate_buffer(
		mam_allocator_t   allocator,
		size_t            size,
		mam_buff_desc_t **desc) {
	mam_error_t err = MAM_SUCCESS;
	mam_buff_desc_t *new_desc =
		calloc(1, sizeof(mam_buff_desc_t));
	MAM_REFUTE(!new_desc, MAM_ENOMEM);

	new_desc->addr = allocator->alloc1.alloc(size);
	MAM_REFUTE_ERR_GOTO(err, !new_desc->addr, MAM_ENOMEM, err_desc);
	new_desc->m_addr = allocator->alloc2.alloc(size);
	MAM_REFUTE_ERR_GOTO(err, !new_desc->m_addr, MAM_ENOMEM, err_addr);
	new_desc->free = size;
	new_desc->size = size;
	allocator->current_buffer = new_desc;
	allocator->total_size += size;
	*desc = new_desc;
	return MAM_SUCCESS;
err_addr:
	allocator->alloc1.free(new_desc->addr);
err_desc:
	free(new_desc);
	return err;
}

static inline bool
_mam_find_in_buffer(
		mam_buff_desc_t  *buffer,
		size_t            size,
		size_t            alignment,
		void            **addr,
		void            **mirror_addr) {
	size_t mask = alignment - 1;
	size_t offset = buffer->size - buffer->free;
	/* pad for the target */
	size_t pad = ((uintptr_t)buffer->m_addr + offset) & mask;
	size_t sz = size;
	if( pad ) {
		pad = alignment - pad;
		sz += pad;
		offset += pad;
	}
	if ( buffer->free > sz ) {
		buffer->free -= sz;
		buffer->count += 1;
		*addr = (char *)buffer->addr + offset;
		*mirror_addr = (char *)buffer->m_addr + offset;
		return true;
	}
	else
		return false;
}

/*
static inline mam_buff_desc_t *
_mam_find_buffer(
		mam_allocator_t   allocator,
		size_t            size,
		size_t            alignment,
		void            **addr,
		void            **mirror_addr) {
	if (allocator->total_size < size)
		return NULL;
	UT_array *array = allocator->buffers;
	mam_buff_desc_t **p_buffer = NULL;
	while ( (p_buffer =  (mam_buff_desc_t **)utarray_next(array, p_buffer)) ) {
		mam_buff_desc_t *buff = *p_buffer;
		if (_mam_find_in_buffer(buff, size, alignment, addr, mirror_addr))
			return buff;
	}
	return NULL;
}*/

#define utarray_oom() { \
	MAM_RAISE(MAM_ENOMEM); \
}

mam_error_t
_mam_allocator_alloc(
		mam_allocator_t   allocator,
		size_t            size,
		size_t            alignment,
		void            **addr,
		void            **mirror_addr,
		mam_buff_desc_t **desc) {
	mam_buff_desc_t *buffer = NULL;
	/* find if the already allocated buffer can accomodate
           the allocation */
	if (allocator->current_buffer) {
		if (_mam_find_in_buffer(allocator->current_buffer, size, alignment, addr, mirror_addr)) {
			buffer = allocator->current_buffer;
			goto found;
		}
		/* buffer full, put in list */
		utarray_push_back(allocator->buffers, &allocator->current_buffer);
		allocator->current_buffer = NULL;
	}
	/* No buffer found, determine buffer alloc size:
	   try reallocating existing size */
	size_t           buff_alloc_sz;
	if (allocator->total_size)
		buff_alloc_sz = allocator->total_size;
	else
		buff_alloc_sz = MAM_ALLOC_SIZE_DEFAULT;
	while (buff_alloc_sz < size)
		buff_alloc_sz <<= 1;
	MAM_VALIDATE(_mam_allocate_buffer(allocator, buff_alloc_sz, &buffer));
	allocator->current_buffer = buffer;
	_mam_find_in_buffer(buffer, size, alignment, addr, mirror_addr);
found:
	if (desc)
		*desc = buffer;
	return MAM_SUCCESS;
}
#undef  utarray_oom

mam_error_t
mam_allocator_alloc(
		mam_allocator_t   allocator,
		size_t            size,
		void            **addr,
		void            **mirror_addr,
		mam_buff_desc_t **desc) {
	MAM_CHECK_PTR(allocator);
	MAM_CHECK_PTR(addr);
	MAM_CHECK_PTR(mirror_addr);
	MAM_REFUTE(!size, MAM_EINVAL);
	MAM_VALIDATE(_mam_allocator_alloc(
		allocator, size, 1, addr, mirror_addr, desc));
	return MAM_SUCCESS;
}

mam_error_t
mam_allocator_alloc_aligned(
		mam_allocator_t   allocator,
		size_t            size,
		size_t            alignment,
		void            **addr,
		void            **mirror_addr,
		mam_buff_desc_t **desc) {
	MAM_CHECK_PTR(allocator);
	MAM_CHECK_PTR(addr);
	MAM_CHECK_PTR(mirror_addr);
	MAM_REFUTE(!size, MAM_EINVAL);
	MAM_REFUTE(alignment & (alignment-1), MAM_EINVAL);
	MAM_VALIDATE(_mam_allocator_alloc(
		allocator, size, alignment, addr, mirror_addr, desc));
	return MAM_SUCCESS;
}

mam_error_t
mam_allocator_get_buffer_descs(
		mam_allocator_t   allocator,
		size_t            n_buff,
		mam_buff_desc_t **descs,
		size_t           *n_buff_ret) {
	MAM_CHECK_PTR(allocator);
	MAM_CHECK_ARY(n_buff, descs);
	size_t len = utarray_len(allocator->buffers);
	size_t count = len;
	if (allocator->current_buffer)
		count += 1;
	if (descs) {
		MAM_REFUTE(n_buff < count, MAM_EINVAL);
		UT_array *array = allocator->buffers;
		for (size_t i = 0; i < len; i++)
			descs[i] = *(mam_buff_desc_t **)utarray_eltptr(array, i);
		if (allocator->current_buffer)
			descs[len] = allocator->current_buffer;
		for (size_t i = count; i < n_buff; i++)
			descs[i] = NULL;
	}
	if (n_buff_ret)
		*n_buff_ret = count;
	return MAM_SUCCESS;
}
