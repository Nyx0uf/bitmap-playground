#ifndef __NYX_IMGWRITER_H__
#define __NYX_IMGWRITER_H__

#include "bitmap.h"


/**
 * @brief Save a bitmap object to a given path with a given type
 * @param filepath [in] : Path to save the file to
 * @param bm [in] : Bitmap
 * @param type [in] : image type to save to (currently only TGA/PNG supported)
 * @param output_colorspace [in] : colorspace to save the image (RGB, RGBA)
 * @returns true if the bitmap was successfully written
 */
bool nyx_img_write_bitmap_to_file(const char* filepath, const bitmap* bm, const img_type_t type, const colorspace_t output_colorspace);


#endif /* __NYX_IMGWRITER_H__ */
