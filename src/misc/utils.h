#ifndef __NYX_UTILS_H__
#define __NYX_UTILS_H__

#include <sys/types.h>


/**
 * @brief Perform an aligned memory allocation
 * @param size [in] : Original size of the allocation
 * @param align [in] : Alignment wanted
 * @returns Pointer to allocated memory
 */
void* nyx_aligned_malloc(const size_t size, const size_t align);

/**
 * @brief Free an aligned memory allocation
 * @param ptr [in] : Memory to free
 */
void nyx_aligned_free(void* ptr);


#endif /* __NYX_UTILS_H__ */
