#include "filter_grayscale.h"
#include "cl/cl_global.h"
#include <math.h>


static const char* kernel_filter_grayscale1 = "\
__kernel void grayscale(__global int* input, __global int* output, const size_t count)\
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
		int lum = (int)((red * 0.2126f) + (green * 0.7152f) + (blue * 0.0722f));\
		lum = clamp(lum, 0, 255);\
\
		output[i] = ((alpha << 24) + (lum << 16) + (lum << 8) + lum);\
	}\
}\
";

static const char* kernel_filter_grayscale2 = "\
__kernel void grayscale(__global int2* input, __global int2* output, const size_t count)\
{\
	const size_t i = get_global_id(0);\
	if (i < count)\
	{\
		const int2 cur_pixel = input[i];\
		int2 red = cur_pixel & 0xFF;\
		int2 green = (cur_pixel >> 8) & 0xFF;\
		int2 blue = (cur_pixel >> 16) & 0xFF;\
		int2 alpha = (cur_pixel >> 24) & 0xFF;\
\
		int2 lum = convert_int2(((convert_float2(red) * (float2)0.2126f) + (convert_float2(green) * (float2)0.7152f) + (convert_float2(blue) * (float2)0.0722f)));\
		lum = clamp(lum, (int2)0, (int2)255);\
\
		output[i] = ((alpha << 24) + (lum << 16) + (lum << 8) + lum);\
	}\
}\
";

static const char* kernel_filter_grayscale4 = "\
__kernel void grayscale(__global int4* input, __global int4* output, const size_t count)\
{\
	const size_t i = get_global_id(0);\
	if (i < count)\
	{\
		const int4 cur_pixel = input[i];\
		int4 red = cur_pixel & 0xFF;\
		int4 green = (cur_pixel >> 8) & 0xFF;\
		int4 blue = (cur_pixel >> 16) & 0xFF;\
		int4 alpha = (cur_pixel >> 24) & 0xFF;\
\
		int4 lum = convert_int4(((convert_float4(red) * (float4)0.2126f) + (convert_float4(green) * (float4)0.7152f) + (convert_float4(blue) * (float4)0.0722f)));\
		lum = clamp(lum, (int4)0, (int4)255);\
\
		output[i] = ((alpha << 24) + (lum << 16) + (lum << 8) + lum);\
	}\
}\
";

static const char* kernel_filter_grayscale8 = "\
__kernel void grayscale(__global int8* input, __global int8* output, const size_t count)\
{\
	const size_t i = get_global_id(0);\
	if (i < count)\
	{\
		const int8 cur_pixel = input[i];\
		int8 red = cur_pixel & 0xFF;\
		int8 green = (cur_pixel >> 8) & 0xFF;\
		int8 blue = (cur_pixel >> 16) & 0xFF;\
		int8 alpha = (cur_pixel >> 24) & 0xFF;\
\
		int8 lum = convert_int8(((convert_float8(red) * (float8)0.2126f) + (convert_float8(green) * (float8)0.7152f) + (convert_float8(blue) * (float8)0.0722f)));\
		lum = clamp(lum, (int8)0, (int8)255);\
\
		output[i] = ((alpha << 24) + (lum << 16) + (lum << 8) + lum);\
	}\
}\
";

static const char* kernel_filter_grayscale16 = "\
__kernel void grayscale(__global int16* input, __global int16* output, const size_t count)\
{\
	const size_t i = get_global_id(0);\
	if (i < count)\
	{\
		/*printf(\"i = %d\\n\");*/\
		const int16 cur_pixel = input[i];\
		int16 red = cur_pixel & 0xFF;\
		int16 green = (cur_pixel >> 8) & 0xFF;\
		int16 blue = (cur_pixel >> 16) & 0xFF;\
		int16 alpha = (cur_pixel >> 24) & 0xFF;\
\
		int16 lum = convert_int16(((convert_float16(red) * (float16)0.2126f) + (convert_float16(green) * (float16)0.7152f) + (convert_float16(blue) * (float16)0.0722f)));\
		lum = clamp(lum, (int16)0, (int16)255);\
\
		output[i] = ((alpha << 24) + (lum << 16) + (lum << 8) + lum);\
	}\
}\
";

/* just stfu clang */
#pragma unused(kernel_filter_grayscale1)
#pragma unused(kernel_filter_grayscale2)
#pragma unused(kernel_filter_grayscale4)
#pragma unused(kernel_filter_grayscale8)
#pragma unused(kernel_filter_grayscale16)


bool nyx_filter_grayscale(const bitmap* bm_in, bitmap* bm_out)
{
	if ((!bm_in) || (!bm_out))
		return false;

	const size_t width = bm_in->width;
	const size_t height = bm_in->height;
	if ((width != bm_out->width) && (height != bm_out->height))
		return false;

	rgba_pixel* in_ptr = (rgba_pixel*)bm_in->buffer, *out_ptr = (rgba_pixel*)bm_out->buffer;
	int lum;
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			lum = (int)((in_ptr->r * 0.2126f) + (in_ptr->g * 0.7152f) + (in_ptr->b * 0.0722f));

			out_ptr->r = out_ptr->g = out_ptr->b = (uint8_t)NYX_SAFE_PIXEL_COMPONENT_VALUE(lum);
			out_ptr->a = in_ptr->a;

			// next pixel
			out_ptr++;
			in_ptr++;
		}
	}

	return true;
}

bool nyx_filter_grayscale_opencl(const bitmap* bm_in, bitmap* bm_out)
{
	if ((!bm_in) || (!bm_out))
		return false;

	const size_t width = bm_in->width;
	const size_t height = bm_in->height;
	if ((width != bm_out->width) && (height != bm_out->height))
		return false;

	const size_t bm_wh = width * height;
	const size_t bm_size = bm_wh * sizeof(int);

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
	const cl_uint vec_width = nyx_cl_get_int_vector_width();
	const size_t wrk_count = (size_t)ceilf(bm_wh / (float)vec_width);

	// create the compute program from the source buffer
	char* filter_kernel = NULL;
	switch (vec_width)
	{
		case 1:
			filter_kernel = (char*)kernel_filter_grayscale1;
			break;
		case 2:
			filter_kernel = (char*)kernel_filter_grayscale2;
			break;
		case 4:
			filter_kernel = (char*)kernel_filter_grayscale4;
			break;
		case 8:
			filter_kernel = (char*)kernel_filter_grayscale8;
			break;
		case 16:
			filter_kernel = (char*)kernel_filter_grayscale16;
			break;
		default:
			filter_kernel = (char*)kernel_filter_grayscale1;
			break;
	}
	program = clCreateProgramWithSource(context, 1, (const char**)&filter_kernel, NULL, &err);
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
	kernel = clCreateKernel(program, "grayscale", &err);
	if ((!kernel) || (err != CL_SUCCESS))
	{
		NYX_ERRLOG("[!] Error: Failed to create compute kernel (%d)\n", err);
		goto out;
	}

	// create the input and output arrays in device memory for our calculation
	input = clCreateBuffer(context, CL_MEM_READ_ONLY, bm_size, NULL, NULL);
	output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bm_size, NULL, NULL);
	if ((!input) || (!output))
	{
		NYX_ERRLOG("[!] Error: Failed to allocate device memory (%d)\n", err);
		goto out;
	}

	// write our data set into the input array in device memory
	err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, bm_size, bm_in->buffer, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		NYX_ERRLOG("[!] Error: Failed to write to source array (%d)\n", err);
		goto out;
	}

	// set the arguments to our compute kernel
	err = 0;
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
	err |= clSetKernelArg(kernel, 2, sizeof(size_t), &wrk_count);
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
	global = wrk_count;
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

	// read back the results from the device
	err = clEnqueueReadBuffer(commands, output, CL_TRUE, 0, bm_size, bm_out->buffer, 0, NULL, NULL);
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
