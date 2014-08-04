#include "filter_sepia.h"
#include "cl/cl_global.h"


static const char* kernel_filter_sepia = "\
__kernel void sepia(__global int* input, __global int* output, const size_t count)\
{\
	const size_t i = get_global_id(0);\
	if (i < count)\
	{\
		const int cur_pixel = input[i];\
		int red = cur_pixel & 0xFF;\
		int green = (cur_pixel >> 8) & 0xFF;\
		int blue = (cur_pixel >> 16) & 0xFF;\
		int alpha = (cur_pixel >> 24) & 0xFF;\
\
		int newRed = fma(red, 0.393f, fma(green, 0.769f, fma(blue, 0.189f, 0.0f)));\
		int newGreen = fma(red, 0.349f, fma(green, 0.686f, fma(blue, 0.168f, 0.0f)));\
		int newBlue = fma(red, 0.272f, fma(green, 0.534f, fma(blue, 0.131f, 0.0f)));\
\
		newRed = clamp(newRed, 0, 255);\
		newGreen = clamp(newGreen, 0, 255);\
		newBlue = clamp(newBlue, 0, 255);\
\
		output[i] = ((alpha << 24) + (newBlue << 16) + (newGreen << 8) + newRed);\
		/*printf(\"index[%d]\\n\", (int)i);*/\
	}\
}\
";

static const char* kernel_filter_sepia_image2d = "\
__kernel void sepia(__read_only image2d_t input, __write_only image2d_t output)\
{\
	const int2 pos = {get_global_id(0), get_global_id(1)};\
	uint4 in = read_imageui(input, pos);\
	uint4 out = (uint4)(fma(in.x, 0.393f, fma(in.y, 0.769f, fma(in.z, 0.189f, 0))),\
						fma(in.x, 0.349f, fma(in.y, 0.686f, fma(in.z, 0.168f, 0))),\
						fma(in.x, 0.272f, fma(in.y, 0.534f, fma(in.z, 0.131f, 0))),\
						in.w);\
	write_imageui(output, pos, out);\
}\
";

/* just stfu clang */
#pragma unused(kernel_filter_sepia_image2d)
#pragma unused(kernel_filter_sepia)


bool nyx_filter_sepia(const bitmap* bm_in, bitmap* bm_out)
{
	if ((!bm_in) || (!bm_out))
		return false;

	const size_t width = bm_in->width;
	const size_t height = bm_in->height;
	if ((width != bm_out->width) && (height != bm_out->height))
		return false;

	rgba_pixel* in_ptr = (rgba_pixel*)bm_in->buffer, *out_ptr = (rgba_pixel*)bm_out->buffer;
	int newRed, newGreen, newBlue;
	uint8_t r, g, b;
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			r = in_ptr->r;
			g = in_ptr->g;
			b = in_ptr->b;
			newRed = (int)((r * 0.393f) + (g * 0.769f) + (b * 0.189f));
			newGreen = (int)((r * 0.349f) + (g * 0.686f) + (b * 0.168f));
			newBlue = (int)((r * 0.272f) + (g * 0.534f) + (b * 0.131f));

			out_ptr->r = (uint8_t)NYX_SAFE_PIXEL_COMPONENT_VALUE(newRed);
			out_ptr->g = (uint8_t)NYX_SAFE_PIXEL_COMPONENT_VALUE(newGreen);
			out_ptr->b = (uint8_t)NYX_SAFE_PIXEL_COMPONENT_VALUE(newBlue);
			out_ptr->a = in_ptr->a;

			// next pixel
			out_ptr++;
			in_ptr++;
		}
	}

	// cleaner/shorter but slower
	/*const size_t size = width * height;
	for (size_t i = 0; i < size; i++)
	{
		r = in_ptr[i].r;
		g = in_ptr[i].g;
		b = in_ptr[i].b;
		newRed = (int)((r * 0.393f) + (g * 0.769f) + (b * 0.189f));
		newGreen = (int)((r * 0.349f) + (g * 0.686f) + (b * 0.168f));
		newBlue = (int)((r * 0.272f) + (g * 0.534f) + (b * 0.131f));

		out_ptr[i].r = (uint8_t)NYX_SAFE_PIXEL_COMPONENT_VALUE(newRed);
		out_ptr[i].g = (uint8_t)NYX_SAFE_PIXEL_COMPONENT_VALUE(newGreen);
		out_ptr[i].b = (uint8_t)NYX_SAFE_PIXEL_COMPONENT_VALUE(newBlue);
		out_ptr[i].a = in_ptr[i].a;
	}*/

	return true;
}

bool nyx_filter_sepia_opencl(const bitmap* bm_in, bitmap* bm_out)
{
	if ((!bm_in) || (!bm_out))
		return false;

	const size_t width = bm_in->width;
	const size_t height = bm_in->height;
	if ((width != bm_out->width) && (height != bm_out->height))
		return false;

	cl_int err;
	size_t global; // global domain size for our calculation
	size_t local; // local domain size for our calculation
	cl_device_id device_id = nyx_cl_get_deviceid();
	cl_context context = nyx_cl_get_context();
	cl_command_queue commands = nyx_cl_get_commandqueue();
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_mem input = NULL; // device memory used for the input array
	cl_mem output = NULL; // device memory used for the output array

	// create the compute program from the source buffer
	program = clCreateProgramWithSource(context, 1, (const char**)&kernel_filter_sepia, NULL, &err);
	if (!program)
	{
		NYX_ERRLOG("[!] Error: Failed to create compute program (%d)\n", err);
		goto out;
	}

	// build the program executable
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		size_t len = 0;
		char reason[2048] = {0x00};
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(reason), reason, &len);
		NYX_ERRLOG("[!] Error: Failed to build program executable (%d):\n%s", err, reason);
		goto out;
	}

	// create the compute kernel in the program we wish to run
	kernel = clCreateKernel(program, "sepia", &err);
	if ((!kernel) || (err != CL_SUCCESS))
	{
		NYX_ERRLOG("[!] Error: Failed to create compute kernel (%d)\n", err);
		goto out;
	}

	// create the input and output arrays in device memory for our calculation
	const size_t size = bm_in->width * bm_in->height;
	input = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * size, NULL, NULL);
	output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * size, NULL, NULL);
	if ((!input) || (!output))
	{
		NYX_ERRLOG("[!] Error: Failed to allocate device memory (%d)\n", err);
		goto out;
	}

	// write our data set into the input array in device memory
	err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(int) * size, bm_in->buffer, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		NYX_ERRLOG("[!] Error: Failed to write to source array (%d)\n", err);
		goto out;
	}

	// set the arguments to our compute kernel
	err = 0;
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
	size_t bla = size * 4;
	err |= clSetKernelArg(kernel, 2, sizeof(size_t), &bla);
	if (err != CL_SUCCESS)
	{
		NYX_ERRLOG("Error: Failed to set kernel arguments (%d)\n", err);
		goto out;
	}

	// get the maximum work group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
	if (err != CL_SUCCESS)
	{
		NYX_ERRLOG("[!] Error: Failed to retrieve kernel work group info (%d)\n", err);
		goto out;
	}

	// execute the kernel over the entire range of our 1d input data set
	// using the maximum number of work group items for this device
	global = size;
	// pad
	while ((global % local) != 0)
		global++;
	err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
	if (err)
	{
		NYX_ERRLOG("[!] Error: Failed to execute kernel (%d)\n", err);
		goto out;
	}

	// wait for the command commands to get serviced before reading back results
	clFinish(commands);

	// read back the results from the device to verify the output
	int* results = (int*)bm_out->buffer;
	err = clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(int) * size, results, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		NYX_ERRLOG("[!] Error: Failed to read output array (%d)\n", err);
	}

out:
    // shutdown and cleanup
	if (input) clReleaseMemObject(input);
	if (output) clReleaseMemObject(output);
	if (program) clReleaseProgram(program);
	if (kernel) clReleaseKernel(kernel);

	return (CL_SUCCESS == err);
}

bool nyx_filter_sepia_opencl2(const bitmap* bm_in, bitmap* bm_out)
{
	if ((!bm_in) || (!bm_out))
		return false;

	const size_t width = bm_in->width;
	const size_t height = bm_in->height;
	if ((width != bm_out->width) && (height != bm_out->height))
		return false;

	cl_int err;
	cl_device_id device_id = nyx_cl_get_deviceid();
	cl_context context = nyx_cl_get_context();
	cl_command_queue commands = nyx_cl_get_commandqueue();
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_mem input = NULL; // device memory used for the input array
	cl_mem output = NULL; // device memory used for the output array

	// create the compute program from the source buffer
	program = clCreateProgramWithSource(context, 1, (const char**)&kernel_filter_sepia_image2d, NULL, &err);
	if (!program)
	{
		NYX_ERRLOG("[!] Error: Failed to create compute program (%d)\n", err);
		goto out;
	}
	
	// build the program executable
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		size_t len = 0;
		char reason[2048] = {0x00};
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(reason), reason, &len);
		NYX_ERRLOG("[!] Error: Failed to build program executable (%d):\n%s", err, reason);
		goto out;
	}
	
	// create the compute kernel in the program we wish to run
	kernel = clCreateKernel(program, "sepia", &err);
	if ((!kernel) || (err != CL_SUCCESS))
	{
		NYX_ERRLOG("[!] Error: Failed to create compute kernel (%d)\n", err);
		goto out;
	}

	// create the input and output arrays in device memory for our calculation
	static const cl_image_format format = {CL_RGBA, CL_UNSIGNED_INT8};
	cl_image_desc desc;
	desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	desc.image_width = bm_in->width;
	desc.image_height = bm_in->height;
	desc.image_depth = 1;
	desc.image_array_size = 1;
	desc.image_row_pitch = 0;//bm_in->width * 4;
	desc.image_slice_pitch = 0;//bm_in->width * bm_in->height * 4;
	desc.num_mip_levels = 0;
	desc.num_samples = 0;
	desc.buffer = NULL;
	input = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc, NULL, &err);
	output = clCreateImage(context, CL_MEM_WRITE_ONLY, &format, &desc, NULL, &err);
	if ((!input) || (!output))
	{
		NYX_ERRLOG("[!] Error: Failed to allocate device memory (%d)\n", err);
		goto out;
	}

	size_t origin[3] = {0};
	size_t region[3] = {bm_in->width, bm_in->height, 1};
	clEnqueueWriteImage(commands, input, CL_TRUE, origin, region, 0, 0, bm_in->buffer, 0, NULL, NULL);

	// set the arguments to our compute kernel
	err = 0;
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
	if (err != CL_SUCCESS)
	{
		NYX_ERRLOG("[!] Error: Failed to set kernel arguments (%d)\n", err);
		goto out;
	}

	size_t gsize[2] = {bm_in->width, bm_in->height};
	//size_t lsize[2] = {16, 16};
	err = clEnqueueNDRangeKernel(commands, kernel, 2, NULL, gsize, NULL, 0, NULL, NULL);
	if (err)
	{
		NYX_ERRLOG("[!] Error: Failed to execute kernel (%d)\n", err);
		goto out;
	}

	// wait for the command commands to get serviced before reading back results
	clFinish(commands);

	// read back the results from the device to verify the output
	err = clEnqueueReadImage(commands, output, CL_TRUE, origin, region, 0, 0, bm_out->buffer, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		NYX_ERRLOG("[!] Error: Failed to read output array (%d)\n", err);
	}

out:
    // shutdown and cleanup
	if (input) clReleaseMemObject(input);
	if (output) clReleaseMemObject(output);
	if (program) clReleaseProgram(program);
	if (kernel) clReleaseKernel(kernel);
	
	return (CL_SUCCESS == err);
}
