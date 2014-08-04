#ifndef __NYX_IMGREADER_H__
#define __NYX_IMGREADER_H__

#include "bitmap.h"


/**
 * @brief Attempt to read and load an image file
 * @param filepath [in] : Path of the file
 * @param out_buffer [out] : Raw image bytes
 * @param out_width [out] : Image width
 * @param out_height [out] : Image height
 * @param out_colorspace [out] : Image colorspace
 * @returns true if the file was successfully read
 */
bool nyx_img_read_file(const char* filepath, uint8_t** out_buffer, size_t* out_width, size_t* out_height, colorspace_t* out_colorspace);


#endif /* __NYX_IMGREADER_H__ */
