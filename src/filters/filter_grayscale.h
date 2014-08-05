#ifndef __NYX_FILTERGRAYSCALE_H__
#define __NYX_FILTERGRAYSCALE_H__

#include "img/bitmap.h"


/**
 * @brief Apply a grayscale filter to a bitmap, both bitmap must have the same width and height
 * @param bm_in [in] : Original bitmap to filter, must not be NULL
 * @param bm_out [out] : Filtered bitmap, must not be NULL
 * @returns true if all OK
 */
bool nyx_filter_grayscale(const bitmap* bm_in, bitmap* bm_out);

/**
 * @brief Apply a grayscale filter to a bitmap using OpenCL, both bitmap must have the same width and height
 * @param bm_in [in] : Original bitmap to filter, must not be NULL
 * @param bm_out [out] : Filtered bitmap, must not be NULL
 * @returns true if all OK
 */
bool nyx_filter_grayscale_opencl(const bitmap* bm_in, bitmap* bm_out);


#endif /* __NYX_FILTERGRAYSCALE_H__ */
