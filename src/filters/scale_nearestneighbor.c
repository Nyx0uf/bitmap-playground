#include "scale_nearestneighbor.h"
#include "cl/cl_global.h"
#include <math.h>


static const char* kernel_filter_scale_nearestneighbor = "\
__kernel void nearestneighbor(__read_only image2d_t input, __write_only image2d_t output, const size_t in_width, const float x_ratio, const float y_ratio)\
{\
	const int2 pos_out = {get_global_id(0), get_global_id(1)};\
	float px = floor(pos_out.x * x_ratio);\
	float py = floor(pos_out.y * y_ratio);\
	const int2 pos_in = {(int)px, (int)py};\
	uint4 in = read_imageui(input, pos_in);\
	write_imageui(output, pos_out, in);\
}\
";


bool nyx_scale_nearestneighbor(const bitmap* bm_in, bitmap* bm_out)
{
	if ((!bm_in) || (!bm_out))
		return false;

	const size_t in_width = bm_in->width;
	const size_t in_height = bm_in->height;
	const size_t out_width = bm_out->width;
	const size_t out_height = bm_out->height;
	int* in_ptr = (int*)bm_in->buffer;
	int* out_ptr = (int*)bm_out->buffer;

	const float x_ratio = in_width / (float)out_width;
	const float y_ratio = in_height / (float)out_height;
	float px, py;
	for (size_t y = 0; y < out_height; y++)
	{
		for (size_t x = 0; x < out_width; x++)
		{
			px = floorf(x * x_ratio);
			py = floorf(y * y_ratio);
			out_ptr[(y * out_width) + x] = in_ptr[(int)((py * in_width) + px)];
		}
	}

	return true;
}

bool nyx_scale_nearestneighbor_opencl(const bitmap* bm_in, bitmap* bm_out)
{
	if ((!bm_in) || (!bm_out))
		return false;

	const size_t in_width = bm_in->width;
	const size_t in_height = bm_in->height;
	const size_t out_width = bm_out->width;
	const size_t out_height = bm_out->height;

	cl_int err;
	cl_device_id device_id = nyx_cl_get_deviceid();
	cl_context context = nyx_cl_get_context();
	cl_command_queue commands = nyx_cl_get_commandqueue();
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_mem input = NULL; // device memory used for the input array
	cl_mem output = NULL; // device memory used for the output array

	// create the compute program from the source buffer
	program = clCreateProgramWithSource(context, 1, (const char**)&kernel_filter_scale_nearestneighbor, NULL, &err);
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
	kernel = clCreateKernel(program, "nearestneighbor", &err);
	if ((!kernel) || (err != CL_SUCCESS))
	{
		NYX_ERRLOG("[!] Error: Failed to create compute kernel (%d)\n", err);
		goto out;
	}

	// create the input and output arrays in device memory for our calculation
	static const cl_image_format format = {CL_RGBA, CL_UNSIGNED_INT8};
	cl_image_desc desc_in;
	desc_in.image_type = CL_MEM_OBJECT_IMAGE2D;
	desc_in.image_width = in_width;
	desc_in.image_height = in_height;
	desc_in.image_depth = 1;
	desc_in.image_array_size = 1;
	desc_in.image_row_pitch = 0;
	desc_in.image_slice_pitch = 0;
	desc_in.num_mip_levels = 0;
	desc_in.num_samples = 0;
	desc_in.buffer = NULL;
	input = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc_in, NULL, &err);
	cl_image_desc desc_out = desc_in;
	desc_out.image_width = out_width;
	desc_out.image_height = out_height;
	output = clCreateImage(context, CL_MEM_WRITE_ONLY, &format, &desc_out, NULL, &err);
	if ((!input) || (!output))
	{
		NYX_ERRLOG("[!] Error: Failed to allocate device memory (%d)\n", err);
		goto out;
	}

	size_t origin[3] = {0};
	size_t region_in[3] = {in_width, in_height, 1};
	clEnqueueWriteImage(commands, input, CL_TRUE, origin, region_in, 0, 0, bm_in->buffer, 0, NULL, NULL);

	// set the arguments to our compute kernel
	err = 0;
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
	err |= clSetKernelArg(kernel, 2, sizeof(size_t), &in_width);
	const float x_ratio = in_width / (float)out_width;
	const float y_ratio = in_height / (float)out_height;
	err |= clSetKernelArg(kernel, 3, sizeof(float), &x_ratio);
	err |= clSetKernelArg(kernel, 4, sizeof(float), &y_ratio);
	if (err != CL_SUCCESS)
	{
		NYX_ERRLOG("[!] Error: Failed to set kernel arguments (%d)\n", err);
		goto out;
	}

	size_t gsize[2] = {out_width, out_height};
	err = clEnqueueNDRangeKernel(commands, kernel, 2, NULL, gsize, NULL, 0, NULL, NULL);
	if (err)
	{
		NYX_ERRLOG("[!] Error: Failed to execute kernel (%d)\n", err);
		goto out;
	}

	// wait for the command commands to get serviced before reading back results
	clFinish(commands);

	// read back the results from the device to verify the output
	size_t region_out[3] = {out_width, out_height, 1};
	err = clEnqueueReadImage(commands, output, CL_TRUE, origin, region_out, 0, 0, bm_out->buffer, 0, NULL, NULL);
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
