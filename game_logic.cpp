#include "game_logic.h"
#include "globals.h"

#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

#define THREADS_PER_BLOCK 8
cl_device_id device_id = NULL;
cl_context context = NULL;
cl_command_queue command_queue = NULL;
cl_mem memobj = NULL;
cl_program program = NULL;
cl_kernel kernel = NULL;
cl_platform_id platform_id = NULL;
cl_uint ret_num_devices;
cl_uint ret_num_platforms;
cl_int ret;
 
int* livesArrayActual;
int N;
int size;

cl_mem d_livesArrayActual;
cl_mem d_livesArrayNext;

int* generateInitialLives(int seed, int aliveCellsSize){
	int* cellIndexes = (int*) malloc(sizeof(int)*aliveCellsSize);

	srand(seed);
	for (int i = 0; i < aliveCellsSize; i++){
		int cellIndex = rand() % (COLUMNS*ROWS);
		bool validRandom = true;

		for (int j = 0; j < i; j++){
			if (cellIndexes[j] == cellIndex){
				// need to pick another random number
				i--;
				validRandom = false;
				break;
			}
		}
		if (validRandom)
			cellIndexes[i] = cellIndex;
	}
	return cellIndexes;

}

void swapLivesArrays(){
	ret = clSetKernelArg(kernel, 0, sizeof(d_livesArrayNext), (void *)&d_livesArrayNext);
	ret = clSetKernelArg(kernel, 1, sizeof(d_livesArrayActual), (void *)&d_livesArrayActual);
}

void init_opencl(){
	FILE *fp;
	char fileName[] = "./kernel.cl";
	char *source_str;
	size_t source_size;
	 
	/* Load the source code containing the kernel*/
	fp = fopen(fileName, "r");
	if (!fp) {
	fprintf(stderr, "Failed to load kernel.\n");
	exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
	 
	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	 
	/* Create OpenCL context */
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	 
	 	/* Create Kernel Program from the source */
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
	(const size_t *)&source_size, &ret);
	 
	/* Build Kernel Program */
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	 
	/* Create OpenCL Kernel */
	kernel = clCreateKernel(program, "refresh_Life", &ret);

     
	/* Create Command Queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	 
	/* Create Memory Buffer */
	
	d_livesArrayActual = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &ret);
	d_livesArrayNext = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &ret); 

	clEnqueueWriteBuffer(command_queue, d_livesArrayActual, CL_TRUE, 0, size, livesArrayActual, 0, NULL, NULL);

	 
	/* Set OpenCL Kernel Parameters */
	ret = clSetKernelArg(kernel, 0, sizeof(d_livesArrayActual), (void *)&d_livesArrayActual);
	ret = clSetKernelArg(kernel, 1, sizeof(d_livesArrayNext), (void *)&d_livesArrayNext);
	
	free(source_str);

}

int* init_game_data(){
	N = COLUMNS*ROWS;
	size = sizeof(int)*N;

	livesArrayActual = (int*) malloc(size);

	int initialAliveCellsSize = (int) COLUMNS*ROWS*0.3;
	int* initialAliveCells = generateInitialLives(1, initialAliveCellsSize);
	for (int i = 0; i < initialAliveCellsSize; i++){
		// initial set up of lives
		livesArrayActual[initialAliveCells[i]] = 1;
	}
	free(initialAliveCells);

	init_opencl();
	return livesArrayActual;
}

void free_opencl_resources(){
	/* Finalization */
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(memobj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
}

float kernel_wrapper(){
	size_t global_item_size = N;
    size_t local_item_size = THREADS_PER_BLOCK;
    cl_event event;
   
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event);
	clWaitForEvents(1, &event);
	
	// SWAP ARRAYS
	swapLivesArrays();
	
	return 0;
}

int* fetch_gpu_data(){
    ret = clEnqueueReadBuffer(command_queue, d_livesArrayNext, CL_TRUE, 0, size, livesArrayActual, 0, NULL, NULL);
	return livesArrayActual;
	}


