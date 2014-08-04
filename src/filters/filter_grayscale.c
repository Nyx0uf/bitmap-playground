#include "filter_grayscale.h"
#include "cl/cl_global.h"


static const char* kernel_filter_grayscale = "\
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

/* just stfu clang */
#pragma unused(kernel_filter_grayscale)


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
	program = clCreateProgramWithSource(context, 1, (const char**)&kernel_filter_grayscale, NULL, &err);
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
