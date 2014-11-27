#ifndef __NYX_CROP_H__
#define __NYX_CROP_H__

#include "img/bitmap.h"


/**
 * @brief Crop a bitmap
 * @param bm_in [in] : Original bitmap to crop, must not be NULL
 * @param crop_rect [in] : Zone to crop
 * @param bm_out [out] : Cropped bitmap, must not be NULL
 * @returns true if all OK
 */
bool nyx_crop(const bitmap* bm_in, const rect crop_rect, bitmap* bm_out);


#endif /* __NYX_CROP_H__ */
