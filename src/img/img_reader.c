#include "img_reader.h"
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <jpeglib.h>


static bool _nyx_img_read_png(FILE* fp, uint8_t** out_data, size_t* out_width, size_t* out_height, colorspace_t* out_colorspace);
static bool _nyx_img_read_jpg(FILE* fp, uint8_t** out_data, size_t* out_width, size_t* out_height, colorspace_t* out_colorspace);
static bool _nyx_img_is_type(const uint8_t* header, const img_type_t type);


bool nyx_img_read_file(const char* filepath, uint8_t** out_buffer, size_t* out_width, size_t* out_height, colorspace_t* out_colorspace)
{
	if (!filepath)
	{
		NYX_ERRLOG("[!] filepath is NULL\n");
		return false;
	}

	// open file and test for its type
	FILE* fp = fopen(filepath, "rb");
	if (!fp)
	{
		NYX_ERRLOG("[!] failed to open <%s>\n", filepath);
		return false;
	}
	uint8_t header[18];
	fread(header, 1, 18, fp);
	// rewind fp
	fseek(fp, 0, SEEK_SET);

	bool (*load_img_fptr)(FILE*, uint8_t**, size_t*, size_t*, colorspace_t*);
	if (_nyx_img_is_type(header, img_type_png))
	{
		// PNG
		load_img_fptr = _nyx_img_read_png;
	}
	else if (_nyx_img_is_type(header, img_type_jpg))
	{
		// JPEG
		load_img_fptr = _nyx_img_read_jpg;
	}
	else
	{
		// unsupported image type
		NYX_ERRLOG("[!] unsupported image type <%s>\n", filepath);
		fclose(fp);
		return false;
	}

	// actually read file and get raw bytes
	const bool ret = load_img_fptr(fp, out_buffer, out_width, out_height, out_colorspace);

	// close the file
	fclose(fp);

	return ret;
}

/*** Private ***/
static bool _nyx_img_read_png(FILE* fp, uint8_t** out_data, size_t* out_width, size_t* out_height, colorspace_t* out_colorspace)
{
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		NYX_ERRLOG("[!] png_create_read_struct()\n");
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		NYX_ERRLOG("[!] png_create_info_struct()\n");
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// if we get here, we had a problem reading the file
		NYX_ERRLOG("[!] png_read_png()\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}

    // setup output control if you are using standard C streams
	png_init_io(png_ptr, fp);

    // read png
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);

	png_uint_32 width, height;
	int bit_depth;
	int color_space, interlace_type;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_space, &interlace_type, NULL, NULL);
	*out_width = width;
	*out_height = height;

	// handle colorspace
	if ((color_space != PNG_COLOR_TYPE_RGBA) && (color_space != PNG_COLOR_TYPE_RGB))
	{
		NYX_DLOG("[!] unsupported colorspace <%d>\n", color_space);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}
	switch (color_space)
	{
		case PNG_COLOR_TYPE_RGB:
			*out_colorspace = colorspace_rgb;
			break;
		case PNG_COLOR_TYPE_RGBA:
			*out_colorspace = colorspace_rgba;
			break;
		default:
			*out_colorspace = colorspace_unknown;
			break;
	}

	// create buffer for the raw bytes
	const png_size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	*out_data = (uint8_t*)malloc(sizeof(uint8_t) * row_bytes * height);
	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
	for (size_t i = 0; i < height; i++)
		memcpy(*out_data + (row_bytes * i), row_pointers[i], row_bytes);

    // cleanup
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return true;
}

static bool _nyx_img_read_jpg(FILE* fp, uint8_t** out_data, size_t* out_width, size_t* out_height, colorspace_t* out_colorspace)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];

	cinfo.err = jpeg_std_error(&jerr);

	// decompress jpeg
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	// handle colorspace
	if (cinfo.out_color_space != JCS_RGB)
	{
		NYX_DLOG("[!] unsupported colorspace <%d>\n", cinfo.out_color_space);
		jpeg_destroy_decompress(&cinfo);
		return false;
	}

	// allocate memory to hold the uncompressed image
	*out_width = cinfo.output_width;
	*out_height = cinfo.output_height;
	*out_colorspace = (cinfo.out_color_components == 3) ? colorspace_rgb : ((cinfo.out_color_components == 4) ? colorspace_rgba : colorspace_unknown);
	const size_t size = cinfo.output_width * cinfo.output_height * (size_t)cinfo.num_components;
	*out_data = (uint8_t*)malloc(sizeof(uint8_t) * size);
	// read the jpeg into the raw buffer
	row_pointer[0] = (uint8_t*)malloc(cinfo.output_width * (size_t)cinfo.num_components);
	// read one scan line at a time
	size_t location = 0;
	const size_t stride = cinfo.image_width * (size_t)cinfo.num_components;
	while (cinfo.output_scanline < cinfo.image_height)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		memcpy((*out_data) + location, row_pointer[0], stride);
		location += stride;
	}

	// cleanup
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(row_pointer[0]);

	return true;
}

/**
 * @brief Verify if a file corresponds to the given type by looking its header
 * @param header [in] : Image header
 * @param type [in] : Type we want to check (JPG, PNG, TGA...)
 * @returns true if the header corresponds to type
 */
static bool _nyx_img_is_type(const uint8_t* header, const img_type_t type)
{
	bool ret = false;
	if (!header)
		return ret;

	switch (type)
	{
		case img_type_png:
			ret = (0 == png_sig_cmp(header, 0, 8));
			break;
		case img_type_jpg:
			ret = ((header[0] == 0xFF) && (header[1] == 0xD8));
			break;
		case img_type_tga:
			ret = false;
			break;
		default:
			ret = false;
			break;
	}
	return ret;
}
