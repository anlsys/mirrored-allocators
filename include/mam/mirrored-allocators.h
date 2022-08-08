#ifndef MAM_MIRRORED_ALLOCATORS_H
#define MAM_MIRRORED_ALLOCATORS_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum mam_error_e {
	MAM_SUCCESS =  0,
	MAM_EINVAL  = -1,
	MAM_ENOMEM  = -2,
	MAM_EFROZEN = -3
};
typedef enum mam_error_e mam_error_t;

typedef struct _mam_allocator_s *mam_allocator_t;

struct mam_buff_desc_s {
	void    *addr;
	void    *m_addr;
	size_t   size;
	size_t   free;
	bool     ready;
	int      count;
};
typedef struct mam_buff_desc_s mam_buff_desc_t;

struct mam_platform_alloc_s {
	void * (*alloc)(size_t sz);
	void (*free)(void *ptr);
};
typedef struct mam_platform_alloc_s mam_platform_alloc_t;

extern mam_error_t
mam_allocator_create(
	mam_allocator_t      *allocator,
	mam_platform_alloc_t *alloc1,
	mam_platform_alloc_t *alloc2);

extern mam_error_t
mam_allocator_destroy(
	mam_allocator_t allocator);

extern mam_error_t
mam_allocator_alloc(
	mam_allocator_t   allocator,
	size_t            size,
	void            **addr,
	void            **mirror_addr,
	mam_buff_desc_t **desc);

extern mam_error_t
mam_allocator_alloc_aligned(
	mam_allocator_t   allocator,
	size_t            size,
	size_t            alignment,
	void            **addr,
	void            **mirror_addr,
	mam_buff_desc_t **desc);

extern mam_error_t
mam_allocator_attach_ready_cb(
	mam_allocator_t   allocator,
	mam_error_t     (*ready_cb)(
		mam_allocator_t  allocator,
		mam_buff_desc_t  desc,
		void            *user_data),
	void             *user_data);

extern mam_error_t
mam_allocator_get_buffer_descs(
	mam_allocator_t   allocator,
	size_t            n_buff,
	mam_buff_desc_t **descs,
	size_t           *n_buff_ret);

#ifdef __cplusplus
}
#endif

#endif //MAM_MIRRORED_ALLOCATORS_H
