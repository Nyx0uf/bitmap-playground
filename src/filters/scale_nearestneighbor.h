#ifndef __NYX_SCALENEARESTNEIGHBOR_H__
#define __NYX_SCALENEARESTNEIGHBOR_H__

#include "img/bitmap.h"


/**
 * @brief Scale a bitmap using a a nearest neighbor algorithm
 * @param bm_in [in] : Original bitmap to scale, must not be NULL
 * @param bm_out [out] : Scaled bitmap, must not be NULL
 * @returns true if all OK
 */
bool nyx_scale_nearestneighbor(const bitmap* bm_in, bitmap* bm_out);

/**
 * @brief Scale a bitmap using a a nearest neighbor algorithm (OpenCL)
 * @param bm_in [in] : Original bitmap to scale, must not be NULL
 * @param bm_out [out] : Scaled bitmap, must not be NULL
 * @returns true if all OK
 */
bool nyx_scale_nearestneighbor_opencl(const bitmap* bm_in, bitmap* bm_out);


#endif /* __NYX_SCALENEARESTNEIGHBOR_H__ */
