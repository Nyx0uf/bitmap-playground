#ifndef __NYX_FILTERSEPIA_H__
#define __NYX_FILTERSEPIA_H__

#include "img/bitmap.h"


/**
 * @brief Apply a sepia filter to a bitmap, both bitmap must have the same width and height
 * @param bm_in [in] : Original bitmap to filter, must not be NULL
 * @param bm_out [out] : Filtered bitmap, must not be NULL
 * @returns true if all OK
 */
bool nyx_filter_sepia(const bitmap* bm_in, bitmap* bm_out);

/**
 * @brief Apply a sepia filter to a bitmap (OpenCL simple int), both bitmap must have the same width and height
 * @param bm_in [in] : Original bitmap to filter, must not be NULL
 * @param bm_out [out] : Filtered bitmap, must not be NULL
 * @returns true if all OK
 */
bool nyx_filter_sepia_opencl(const bitmap* bm_in, bitmap* bm_out);

/**
 * @brief Apply a sepia filter to a bitmap (OpenCL with image object), both bitmap must have the same width and height
 * @param bm_in [in] : Original bitmap to filter, must not be NULL
 * @param bm_out [out] : Filtered bitmap, must not be NULL
 * @returns true if all OK
 */
bool nyx_filter_sepia_opencl2(const bitmap* bm_in, bitmap* bm_out);


#endif /* __NYX_FILTERSEPIA_H__ */
