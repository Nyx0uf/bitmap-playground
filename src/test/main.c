#include <time.h>
#include <dirent.h>
#include <string.h>
#include "cl/cl_global.h"
#include "filters/filter_sepia.h"
#include "filters/scale_bilinear.h"
#include "filters/scale_nearestneighbor.h"
#include "img/img_writer.h"


int main(int argc, const char* argv[])
{
#pragma unused(argc)
#pragma unused(argv)
	int ret = 0;
	bitmap* bm_in = NULL, *bm_out = NULL;

	if (!nyx_cl_init())
	{
		NYX_ERRLOG("[!] failed to init OpenCL\n");
		ret = -1;
		goto out;
	}

	bm_in = nyx_bm_create_from_file("/Users/nyxouf/Desktop/bla1.png");
	//bm_in = nyx_bm_create_from_file("/Users/nyxouf/Desktop/bla4.jpg");
	if (!bm_in)
	{
		NYX_ERRLOG("[!] no bitmap data.\n");
		ret = -2;
		goto out;
	}

	//bm_out = nyx_bm_alloc(bm_in->width, bm_in->height, NULL);
	bm_out = nyx_bm_alloc(bm_in->width / 2, bm_in->height / 2, NULL);
	if (!bm_out)
	{
		NYX_ERRLOG("[!] Can't alloc bitmap out\n");
		ret = -3;
		goto out;
	}
	
	clock_t begin = 0, end = 0;
	bool ok = false;
	
	begin = clock();
	for (size_t i = 0; i < 10; i++)
	{
		//ok = nyx_filter_sepia(bm_in, bm_out);
		//ok = nyx_filter_sepia_opencl(bm_in, bm_out);

		//ok = nyx_scale_nearestneighbor(bm_in, bm_out);
		ok = nyx_scale_nearestneighbor_opencl(bm_in, bm_out);

		//ok = nyx_scale_bilinear(bm_in, bm_out);
	}
	end = clock();
	fprintf(stdout, "[+] Time: %fs (%d)\n", ((double)(end - begin) / CLOCKS_PER_SEC), (int)ok);
	
	//nyx_img_write_bitmap_to_file("/Users/nyxouf/Desktop/_out1.png", bm_out, img_type_png, colorspace_rgba);
	//nyx_img_write_bitmap_to_file("/Users/nyxouf/Desktop/_out1.jpg", bm_out, img_type_jpg, colorspace_rgb);
	nyx_img_write_bitmap_to_file("/Users/nyxouf/Desktop/_out1.tga", bm_out, img_type_tga, colorspace_rgba);

out:
	nyx_bm_destroy(bm_out);
	nyx_bm_destroy(bm_in);
	
	nyx_cl_destroy();
	
	return ret;
}
