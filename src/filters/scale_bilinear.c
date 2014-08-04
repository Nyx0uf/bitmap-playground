#include "scale_bilinear.h"


bool nyx_scale_bilinear(const bitmap* bm_in, bitmap* bm_out)
{
	if ((!bm_in) || (!bm_out))
		return false;

	const size_t in_width = bm_in->width;
	const size_t in_height = bm_in->height;
	const size_t out_width = bm_out->width;
	const size_t out_height = bm_out->height;
	int* in_ptr = (int*)bm_in->buffer;
	int* out_ptr = (int*)bm_out->buffer;

	int a, b, c, d;
	const float x_ratio = ((float)(in_width - 1)) / out_width;
	const float y_ratio = ((float)(in_height - 1)) / out_height;
	float x_diff, y_diff, xy_diff, mx_diff, my_diff;
	int blue, red, green, alpha;
	size_t offset = 0, index = 0, i, j;
	for (size_t y = 0; y < out_height; y++)
	{
		for (size_t x = 0; x < out_width; x++)
		{
			// formula, where c is a single pixel component (r,g,b,a)
			// C' = aC(1 - w)(1 - h) + bC(w)(1 - h) + cC(h)(1 - w) + dC(wh)
			i = (size_t)(x_ratio * x);
			j = (size_t)(y_ratio * y);
			index = (j * in_width + i);

			a = in_ptr[index];
			b = in_ptr[index + 1];
			c = in_ptr[index + in_width];
			d = in_ptr[index + in_width + 1];

			x_diff = (x_ratio * x) - i;
			y_diff = (y_ratio * y) - j;
			xy_diff = (x_diff * y_diff);
			mx_diff = (1 - x_diff);
			my_diff = (1 - y_diff);

			red = (int)(NYX_RGBA_GET_R(a) * mx_diff * my_diff + NYX_RGBA_GET_R(b) * (x_diff) * my_diff + NYX_RGBA_GET_R(c) * (y_diff) * mx_diff + NYX_RGBA_GET_R(d) * xy_diff);
			green = (int)(NYX_RGBA_GET_G(a) * mx_diff * my_diff + NYX_RGBA_GET_G(b) * (x_diff) * my_diff + NYX_RGBA_GET_G(c) * (y_diff) * mx_diff + NYX_RGBA_GET_G(d) * xy_diff);
			blue = (int)(NYX_RGBA_GET_B(a) * mx_diff * my_diff + NYX_RGBA_GET_B(b) * (x_diff) * my_diff + NYX_RGBA_GET_B(c) * (y_diff) * mx_diff + NYX_RGBA_GET_B(d) * xy_diff);
			alpha = (int)(NYX_RGBA_GET_A(a) * mx_diff * my_diff + NYX_RGBA_GET_A(b) * (x_diff) * my_diff + NYX_RGBA_GET_A(c) * (y_diff) * mx_diff + NYX_RGBA_GET_A(d) * xy_diff);

			out_ptr[offset++] = (int)NYX_RGBA_MAKE(red, green, blue, alpha);
		}
	}

	return true;
}
