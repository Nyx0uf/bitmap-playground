#include "crop.h"


bool nyx_crop(const bitmap* bm_in, const rect crop_rect, bitmap* bm_out)
{
	if ((!bm_in) || (!bm_out))
		return false;

	// Check if the cropped rect doesn't overflow from the original bitmap
	const size_t xw = NYX_RECT_GET_MAX_X(crop_rect);
	const size_t yh = NYX_RECT_GET_MAX_Y(crop_rect);
	const size_t width = bm_in->width;
	if ((xw > width) || (yh > bm_in->height))
		return false;

	// If the cropped rect is not the same size as the out bitmap size, we have a problem
	const size tmp_s = (size){.w = bm_out->width, .h = bm_out->height};
	if (!NYX_EQUAL_SIZES(tmp_s, crop_rect.size))
		return false;
	
	rgba_pixel* in_ptr = (rgba_pixel*)bm_in->buffer, *out_ptr = (rgba_pixel*)bm_out->buffer;
	for (size_t y = crop_rect.origin.y; y < yh; y++)
	{
		for (size_t x = crop_rect.origin.x; x < xw; x++)
		{
			size_t index = x + y * width;
			*out_ptr++ = in_ptr[index];
		}
	}

	return true;
}
