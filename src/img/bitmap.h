#ifndef __NYX_BITMAP_H__
#define __NYX_BITMAP_H__

#include "misc/global.h"


/* Bitmap */
typedef struct _nyx_bitmap_struct
{
	void* buffer;
	size_t width;
	size_t height;
	size_t stride;
} bitmap;

/*** Bitmap memory management ***/

/**
 * @brief Create a bitmap object
 * @param width [in] : bitmap width
 * @param height [in] : bitmap height
 * @param data [in] : {OPTIONAL} Pointer to bitmap data
 * @returns the bitmap, NULL if memory alloc failed
 */
bitmap* nyx_bm_alloc(const size_t width, const size_t height, const void* data);

/**
 * @brief free the bitmap and its buffer
 * @param bm [in] : bitmap object to destroy
 */
void nyx_bm_destroy(bitmap* bm);

/**
 * @brief Make a copy of a bitmap object
 * @param src [in] : bitmap object to copy
 * @returns A copy of src, or NULL if there was a malloc error
 */
bitmap* nyx_bm_copy(const bitmap* src);

/*** Bitmap I/O ***/

/**
 * @brief Create a bitmap object from a filepath
 * @param filepath [in] : Path of the file
 * @returns the bitmap, NULL failed
 */
bitmap* nyx_bm_create_from_file(const char* filepath);


#endif /* __NYX_BITMAP_H__ */
