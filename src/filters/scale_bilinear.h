#ifndef __NYX_SCALEBILINEAR_H__
#define __NYX_SCALEBILINEAR_H__

#include "img/bitmap.h"


/**
 * @brief Scale a bitmap using a bilinear algorithm, scaling more than 2x or -2x will be ugly
 * @param bm_in [in] : Original bitmap to scale, must not be NULL
 * @param bm_out [out] : Scaled bitmap, must not be NULL
 * @returns true if all OK
 */
bool nyx_scale_bilinear(const bitmap* bm_in, bitmap* bm_out);


#endif /* __NYX_SCALEBILINEAR_H__ */
