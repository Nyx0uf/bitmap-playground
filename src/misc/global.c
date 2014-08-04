#include "global.h"


size_t nyx_num_components_for_colorspace(const colorspace_t colorspace)
{
	size_t num_components = 0;
	switch (colorspace)
	{
		case colorspace_rgb:
			num_components = 3;
			break;
		case colorspace_rgba:
			num_components = 4;
			break;
		default:
			num_components = 0;
			break;
	}
	return num_components;
}
