#include "bitmap.h"
#include <stdlib.h>
#include <string.h>
#include "img_reader.h"

#ifdef NYX_USE_ALIGNED_ALLOCATIONS
#include "misc/utils.h"
#define NYX_MEM_ALIGN 64
#endif


/*** Bitmap memory management ***/
bitmap* nyx_bm_alloc(const size_t width, const size_t height, const void* data)
{
	// alloc bitmap
	bitmap* bm = (bitmap*)malloc(sizeof(bitmap));
	if (!bm)
		return NULL;

	// alloc underlying buffer
	const size_t stride = width * 4;
	const size_t size = stride * height;
#ifdef NYX_USE_ALIGNED_ALLOCATIONS
    bm->buffer = nyx_aligned_malloc(size, NYX_MEM_ALIGN);
#else
	bm->buffer = calloc(size, sizeof(uint8_t));
#endif
	// if the alloc failed, useless to continue
	if (!bm->buffer)
	{
		free(bm);
		return NULL;
	}

	bm->width = width;
	bm->height = height;
	bm->stride = stride;
	if (data)
		memcpy(bm->buffer, data, size);
	
	return bm;
}

void nyx_bm_destroy(bitmap* bm)
{
	if (bm)
	{
#ifdef NYX_USE_ALIGNED_ALLOCATIONS
		nyx_aligned_free(bm->buffer);
#else
		free(bm->buffer);
#endif
		free(bm);
	}
}

bitmap* nyx_bm_copy(const bitmap* src)
{
	bitmap* dst = nyx_bm_alloc(src->width, src->height, src->buffer);
	return dst;
}

/*** Bitmap I/O ***/
bitmap* nyx_bm_create_from_file(const char* filepath)
{
	// load image
	uint8_t* buffer = NULL;
	size_t width = 0, height = 0;
	colorspace_t color_space = colorspace_unknown;
	const bool ret = nyx_img_read_file(filepath, &buffer, &width, &height, &color_space);

	bitmap* bm = NULL;
	if ((ret) && (buffer != NULL) && (width > 0) && (height > 0) && (color_space != colorspace_unknown))
	{
		if (colorspace_rgba == color_space)
		{
			// Same colorspace
			bm = nyx_bm_alloc(width, height, buffer);
			//memcpy(bm->buffer, buffer, bm->stride * height);
		}
		else if (colorspace_rgb == color_space)
		{
			// bitmap struct only support RGBA so convert
			rgba_pixel* pixels = (rgba_pixel*)malloc(sizeof(rgba_pixel) * width * height);
			const size_t num_components = nyx_num_components_for_colorspace(color_space);
			const size_t tmp_siz = (width * height * num_components);
			for (size_t i = 0, ct = 0; i < tmp_siz; i += num_components, ++ct)
			{
				pixels[ct].r = buffer[i];
				pixels[ct].g = buffer[i + 1];
				pixels[ct].b = buffer[i + 2];
				pixels[ct].a = NYX_MAX_PIXEL_COMPONENT_VALUE;
			}
			bm = nyx_bm_alloc(width, height, (void*)pixels);
			//memcpy(bm->buffer, pixels, bm->stride * height);
			free(pixels);
		}
	}
	free(buffer);

	return bm;
}
