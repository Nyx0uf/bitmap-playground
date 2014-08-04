#include "img_writer.h"
#include <stdlib.h>
#include <png.h>
#include <jpeglib.h>


static bool _nyx_img_write_tga(const char* filepath, const bitmap* bm, const colorspace_t output_colorspace);
static bool _nyx_img_write_png(const char* filepath, const bitmap* bm, const colorspace_t output_colorspace);
static bool _nyx_img_write_jpg(const char* filepath, const bitmap* bm);


bool nyx_img_write_bitmap_to_file(const char* filepath, const bitmap* bm, const img_type_t type, const colorspace_t output_colorspace)
{
	bool ret = false;

	// Sanity checks
	if ((!filepath) || (!bm))
		return ret;

	// Unsupported colorspace
	if ((output_colorspace != colorspace_rgba) && (output_colorspace != colorspace_rgb))
		return ret;

	switch (type)
	{
		case img_type_tga:
			ret = _nyx_img_write_tga(filepath, bm, output_colorspace);
			break;
		case img_type_png:
			ret = _nyx_img_write_png(filepath, bm, output_colorspace);
			break;
		case img_type_jpg:
			ret = _nyx_img_write_jpg(filepath, bm);
			break;
		default:
			ret = false;
			break;
	}
	return ret;
}

/*** Private ***/
static bool _nyx_img_write_tga(const char* filepath, const bitmap* bm, const colorspace_t output_colorspace)
{
	bool ret = false;
	FILE* fp = fopen(filepath, "wb");
	if (!fp)
		return ret;

	// TGA Header
	uint8_t header[18] = {0};
	header[2] = 2; // RGB
	header[12] = bm->width & 0xFF;
	header[13] = (bm->width >> 8) & 0xFF;
	header[14] = bm->height & 0xFF;
	header[15] = (bm->height >> 8) & 0xFF;
	header[16] = 8 * (uint8_t)nyx_num_components_for_colorspace(output_colorspace); // bits per pixel
	fwrite(header, sizeof(uint8_t), 18, fp);
	
	if (colorspace_rgba == output_colorspace)
	{
		// same colorspace
		const rgba_pixel* pixels = (rgba_pixel*)bm->buffer;
		for (int y = (int)bm->height - 1; y >= 0; y--)
		{
			for (size_t x = 0; x < bm->width; x++)
			{
				const rgba_pixel pixel = pixels[((size_t)y * bm->width) + x];
				fwrite((uint8_t[4]){pixel.b, pixel.g, pixel.r, pixel.a}, sizeof(uint8_t), 4, fp);
			}
		}
		ret = true;
	}
	else if (colorspace_rgb == output_colorspace)
	{
		// bitmap is RGBA and we want RGB24
		const rgba_pixel* pixels = (rgba_pixel*)bm->buffer;
		for (int y = (int)bm->height - 1; y >= 0; y--)
		{
			for (size_t x = 0; x < bm->width; x++)
			{
				const rgba_pixel pixel = pixels[((size_t)y * bm->width) + x];
				if (NYX_MIN_PIXEL_COMPONENT_VALUE == pixel.a)
				{
					// replace the transparency by white
					fwrite((uint8_t[3]){NYX_MAX_PIXEL_COMPONENT_VALUE, NYX_MAX_PIXEL_COMPONENT_VALUE, NYX_MAX_PIXEL_COMPONENT_VALUE}, sizeof(uint8_t), 3, fp);
				}
				else
				{
					fwrite((uint8_t[3]){pixel.b, pixel.g, pixel.r}, sizeof(uint8_t), 3, fp);
				}
			}
		}
		ret = true;
	}

	// cleanup
	fclose(fp);

	return ret;
}

static bool _nyx_img_write_png(const char* filepath, const bitmap* bm, const colorspace_t output_colorspace)
{
	FILE* fp = fopen(filepath, "wb");
	if (!fp)
		return false;

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return false;
	}

	// error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	// set image attributes
	const png_byte bit_depth = 8;
	const png_byte color_type = (colorspace_rgba == output_colorspace) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
	png_set_IHDR(png_ptr, info_ptr, (png_uint_32)bm->width, (png_uint_32)bm->height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	const size_t pixel_size = nyx_num_components_for_colorspace(output_colorspace);
	png_byte** row_pointers = NULL;
	if (colorspace_rgba == output_colorspace)
	{
		// same colorspace
		const rgba_pixel* pixels = (rgba_pixel*)bm->buffer;
		row_pointers = png_malloc(png_ptr, bm->height * sizeof(png_byte*));
		for (size_t y = 0; y < bm->height; ++y)
		{
			png_byte* row = png_malloc(png_ptr, sizeof (uint8_t) * bm->width * pixel_size);
			row_pointers[y] = row;
			for (size_t x = 0; x < bm->width; ++x)
			{
				const rgba_pixel pixel = pixels[((size_t)y * bm->width) + x];
				*row++ = pixel.r;
				*row++ = pixel.g;
				*row++ = pixel.b;
				*row++ = pixel.a;
			}
		}
	}
	else
	{
		// bitmap is RGBA and we want RGB24
		const rgba_pixel* pixels = (rgba_pixel*)bm->buffer;
		row_pointers = png_malloc(png_ptr, bm->height * sizeof(png_byte*));
		for (size_t y = 0; y < bm->height; ++y)
		{
			png_byte* row = png_malloc(png_ptr, sizeof (uint8_t) * bm->width * pixel_size);
			row_pointers[y] = row;
			for (size_t x = 0; x < bm->width; ++x)
			{
				const rgba_pixel pixel = pixels[((size_t)y * bm->width) + x];
				if (NYX_MIN_PIXEL_COMPONENT_VALUE == pixel.a)
				{
					// replace the transparency by white
					*row++ = NYX_MAX_PIXEL_COMPONENT_VALUE;
					*row++ = NYX_MAX_PIXEL_COMPONENT_VALUE;
					*row++ = NYX_MAX_PIXEL_COMPONENT_VALUE;
				}
				else
				{
					*row++ = pixel.r;
					*row++ = pixel.g;
					*row++ = pixel.b;
				}
			}
		}
	}

	// write image data
	png_init_io(png_ptr, fp);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	// cleanup
	for (size_t y = 0; y < bm->height; y++)
		png_free(png_ptr, row_pointers[y]);
	png_free(png_ptr, row_pointers);

	return true;
}

static bool _nyx_img_write_jpg(const char* filepath, const bitmap* bm)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE* fp = fopen(filepath, "wb");
	if (!fp)
		return false;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);

	// set parameters for compression
	cinfo.image_width = (JDIMENSION)bm->width;
	cinfo.image_height = (JDIMENSION)bm->height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 100, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	// need to convert from RGBA to RGB24
	uint8_t* buffer = bm->buffer;
	const size_t stride = bm->stride;
	while (cinfo.next_scanline < cinfo.image_height)
	{
		rgba_pixel* rgba_row = (rgba_pixel*)(&buffer[cinfo.next_scanline * stride]);
		rgb_pixel* rgb_row = (rgb_pixel*)malloc(sizeof(rgb_pixel) * cinfo.image_width);
		for (size_t i = 0; i < cinfo.image_width; i++)
		{
			if (NYX_MIN_PIXEL_COMPONENT_VALUE == rgba_row[i].a)
			{
				// replace the transparency by white
				rgb_row[i].r = rgb_row[i].g = rgb_row[i].b = NYX_MAX_PIXEL_COMPONENT_VALUE;
			}
			else
			{
				rgb_row[i].r = rgba_row[i].r;
				rgb_row[i].g = rgba_row[i].g;
				rgb_row[i].b = rgba_row[i].b;
			}
		}
		(void)jpeg_write_scanlines(&cinfo, (JSAMPROW[1]){(JSAMPROW)rgb_row}, 1);
		free(rgb_row);
	}

	// cleanup
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(fp);

	return true;
}
