//============================================================================
// Name        : controller.cpp
// Author      : Vicente Oyanedel Muñoz
// Version     : 1.8
// Copyright   : MIT (c) 2017
// Description : Game of life parallely implemented, using CUDA, C and OpenGL.
//============================================================================
// ALL
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <iostream>
// GRAPHICS
/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>
/* Using SDL2 for the base window and OpenGL context init */
#include <SDL.h>

// GAME LOGIC
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

// INTERFACES
#include "globals.h"
#include "controller.h"
#include "game_logic.h"
#include "shader_tools.h"

#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

// SHADER TOOLS
extern char* file_read(const char* filename);
extern void print_log(GLuint object);
extern GLuint create_shader(const char* filename, GLenum type);

// GAME LOGIC
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


// CONTROLLER
GLuint glprogram;
GLuint vbo_triangle;
GLuint vao_triangle;
GLint attribute_coord2d;
GLint attribute_color;

//vertices + colores
container_t* vertex_n_colors;

//metricas
unsigned int iteration_counter = 0;
double seconds_of_process = 0;

int win_width = WIDTH;
int win_height = HEIGHT;

using namespace std;

/*************************************************************************************************************************************************************************************/
/** CONTROLLER **/
/*************************************************************************************************************************************************************************************/
/**
* transforma arreglo de vidas a arreglo de vertices de triangulos con sus respectivos colores para dibujar el juego.
* Retorna referencia a contenedor con arreglo de vertices + colores e información de longitudes.
*
*/

container_t* lives_array_to_bw_squares_vertices() {
	double start_x = -1;
	double start_y = -1;

	int first_call = 0;

	double square_ancho = 2.0 / COLUMNS;
	double square_alto = 2.0 / ROWS;
	int n_triangles = COLUMNS * ROWS * 2;
	int array_len = n_triangles * 15; // 3 vertex = 6 (x,y) points + 3 colores per vertex (9) = 15
	if (vertex_n_colors == NULL) {
		float* vertex_n_colors_array = (float*) malloc(
				sizeof(float) * array_len);
		vertex_n_colors = (container_t*) malloc(sizeof(container_t));
		vertex_n_colors->array = vertex_n_colors_array;
		vertex_n_colors->total_len = array_len;
		vertex_n_colors->vertex_len = n_triangles * 3;
		first_call = 1;
	}
	float* vertex_n_colors_array = vertex_n_colors->array;

	if (vertex_n_colors == NULL) {
		printf("error allocating vertex\n");
		return 0;
	}
	int sqr_counter = 0;
	int trngl_counter = 0;
	int coord_counter = 0;

	for (start_x = -1; start_x <= 1 - square_ancho;) {
		double izq = start_x;
		double der = start_x + square_ancho;
		for (start_y = -1; start_y <= 1 - square_alto;) {
			double abajo = start_y;
			double arriba = start_y + square_alto;
			float color;
			if (livesArrayActual[sqr_counter] == 1) {
				color = 1.0f;
			} else {
				color = 0.0f;
			}
			if (first_call) {
				// llenar vertices y colores
				// triangulo 1
				vertex_n_colors_array[coord_counter++] = abajo;
				vertex_n_colors_array[coord_counter++] = izq;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color

				vertex_n_colors_array[coord_counter++] = abajo;
				vertex_n_colors_array[coord_counter++] = der;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color

				vertex_n_colors_array[coord_counter++] = arriba;
				vertex_n_colors_array[coord_counter++] = izq;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color

				//color

				trngl_counter++;
				// triangulo 2
				vertex_n_colors_array[coord_counter++] = abajo;
				vertex_n_colors_array[coord_counter++] = der;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color

				vertex_n_colors_array[coord_counter++] = arriba;
				vertex_n_colors_array[coord_counter++] = der;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color

				vertex_n_colors_array[coord_counter++] = arriba;
				vertex_n_colors_array[coord_counter++] = izq;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				trngl_counter++;
				sqr_counter++;
			} else {
				// llenar solo colores
				// triangulo 1
				coord_counter += 2;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				coord_counter += 2;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				coord_counter += 2;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				trngl_counter++;

				// triangulo 2
				coord_counter += 2;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				coord_counter += 2;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				coord_counter += 2;
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				vertex_n_colors_array[coord_counter++] = color; //color
				trngl_counter++;
				sqr_counter++;
			}

			start_y += square_ancho;
		}
		start_x += square_alto;
	}
	return vertex_n_colors;
}

/**
 * Inicia shaders y calcula vectores y colres iniciales.
 *
 * */
bool init_resources(void) {
	/** SHADER COMPILATION & LINK**/
	GLint compile_ok = GL_FALSE, link_ok = GL_FALSE;
	GLuint vs, fs;
	if ((vs = create_shader("shaders/gameoflife.v.glsl", GL_VERTEX_SHADER))
			== 0)
		return false;
	if ((fs = create_shader("shaders/gameoflife.f.glsl", GL_FRAGMENT_SHADER))
			== 0)
		return false;

	glprogram = glCreateProgram();
	glAttachShader(glprogram, vs);
	glAttachShader(glprogram, fs);
	glLinkProgram(glprogram);
	glGetProgramiv(glprogram, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		cerr << "Error in glLinkProgram" << endl;
		return false;
	}

	const char* attribute_name = "coord2d";
	attribute_coord2d = glGetAttribLocation(glprogram, attribute_name);
	attribute_name = "color";
	attribute_color = glGetAttribLocation(glprogram, attribute_name);

	if (attribute_coord2d == -1) {
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	glViewport(0, 0, win_width, win_height);

	/** VERTEX CALCULATION AND SYFF **/
	vertex_n_colors = lives_array_to_bw_squares_vertices();

	glGenBuffers(1, &vbo_triangle);
	glGenVertexArrays(1, &vao_triangle);
	glBindVertexArray(vao_triangle);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_n_colors->total_len,
			vertex_n_colors->array, GL_STATIC_DRAW);

	/* Describe our vertices array to OpenGL (it can't guess its format automatically) */
	glVertexAttribPointer(attribute_coord2d, // attribute
			2,                 // number of elements per vertex, here (x,y)
			GL_FLOAT,          // the type of each element
			GL_FALSE,          // take our values as-is
			5 * sizeof(float),   //bytes until next element
			(void*) 0  // offset
			);

	glEnableVertexAttribArray(attribute_coord2d);

	glVertexAttribPointer(attribute_color, // attribute
			3,                 // number of elements per vertex, here (x,y,z)
			GL_FLOAT,          // the type of each element
			GL_FALSE,          // take our values as-is
			5 * sizeof(float),            //bytes until next element
			(void*) (2 * sizeof(float)) // offset
			);
	glEnableVertexAttribArray(attribute_color);

	glUseProgram(glprogram);
	return true;
}

/**
 *
 * Calcula nuevos vertices y colores y los pasa a la gpu, luego dibuja.*/
void render(SDL_Window* window) {

	/* Clear the background as white */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, win_width, win_height);

	vertex_n_colors = lives_array_to_bw_squares_vertices();

	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_n_colors->total_len,
			vertex_n_colors->array, GL_STATIC_DRAW);
	float spf = 1.0 / FRAMERATE;
	usleep(1000 * 1000 * spf); // FRAMERATE

	/* Push each element in buffer_vertices to the vertex shader */
	glBindVertexArray(vao_triangle);
	glDrawArrays(GL_TRIANGLES, 0, vertex_n_colors->vertex_len);

	//glDisableVertexAttribArray(attribute_coord2d);
	//glDisableVertexAttribArray(attribute_color);

	/* Display the result */
	SDL_GL_SwapWindow(window);
}

/**
 *
 *
 * Loop del juego de la vida.
 * */
void mainLoop(SDL_Window* window) {
	while (1) {
		// display stuff
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				return;
		}
		render(window); // transfer previous life information to GPU for display and draw
		float milliseconds = kernel_wrapper(); // blocks until gpu finishes proccesing kernel
		livesArrayActual = fetch_gpu_data(); // transfer new life information to CPU
		
		seconds_of_process += milliseconds/1000;
		iteration_counter++;
	}
}
/**
 * Inicia ventana y recursos.
 * Retorna referencia a ventana.
 *
 * */
SDL_Window* init_display_stuff() {
	/* SDL-related initialising functions */
	SDL_Init (SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Game of Life: OpenCL",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_width,
			win_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(window);

	/* Extension wrangler initialising */
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK) {
		cerr << "Error: glewInit: " << glewGetErrorString(glew_status) << endl;
		exit(-2);
	}

	/* When all init functions run without errors,
	 the glprogram can initialise the resources */
	if (!init_resources()) {
		cerr << "Error: init_resources: " << glewGetErrorString(glew_status)
				<< endl;
		exit(-3);

	}
	return window;

}
void free_resources() {
	glDeleteProgram(glprogram);
	glDeleteBuffers(1, &vbo_triangle);
}
void intHandler(int dummy) {
	float res = ((iteration_counter*1.0)/seconds_of_process)*COLUMNS*ROWS;
	printf("\n///////////////// ESTADISTICAS /////////////////////\n");
	printf("- Celulas evaluadas: %d celulas\n", iteration_counter*COLUMNS*ROWS);
	printf("- Segundos evaluando celulas: %f segundos\n", seconds_of_process);
	printf("- Celulas evaluadas por segundo: %.f celulas/segundos\n", res);
	printf("- Tamano de grilla: %d celdas (%d x %d)\n", COLUMNS*ROWS, COLUMNS, ROWS);
	printf("HASTA PRONTO!\n");
	free_resources();
	exit(1);
}



int main() {
    signal(SIGINT, intHandler);
	/** INICIA DATOS DEL JUEGO **/
	livesArrayActual = init_game_data();
	/** INICIA DISPLAY **/
	SDL_Window* window = init_display_stuff();
	/** CORRE VIDA */
	mainLoop(window);

	/* If the glprogram exits in the usual way,
	 free resources and exit with a success */
	free_resources();
	free_opencl_resources();
	return EXIT_SUCCESS;
}
/*************************************************************************************************************************************************************************************/
/** GAME LOGIC **//** GAME LOGIC **//** GAME LOGIC **//** GAME LOGIC **//** GAME LOGIC **//** GAME LOGIC **//** GAME LOGIC **//** GAME LOGIC **/
/*************************************************************************************************************************************************************************************/
// PRIVATE
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

// PUBLIC
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
	kernel = clCreateKernel(program, "refresh_life", &ret);

     
	/* Create Command Queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	 
	/* Create Memory Buffer */
	
	d_livesArrayActual = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &ret);
	d_livesArrayNext = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &ret); 
	cl_mem d_rows = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &ret); 
	cl_mem d_columns = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &ret); 

	int rows = ROWS;
	int columns = COLUMNS;

	/* Copy to Memory Buffer */
	clEnqueueWriteBuffer(command_queue, d_livesArrayActual, CL_TRUE, 0, size, livesArrayActual, 0, NULL, NULL);
	clEnqueueWriteBuffer(command_queue, d_rows, CL_TRUE, 0, sizeof(int), &rows, 0, NULL, NULL);
	clEnqueueWriteBuffer(command_queue, d_columns, CL_TRUE, 0, sizeof(int), &columns, 0, NULL, NULL);
	
	/* Set OpenCL Kernel Parameters */
	ret = clSetKernelArg(kernel, 0, sizeof(d_livesArrayActual), (void *)&d_livesArrayActual);
	ret = clSetKernelArg(kernel, 1, sizeof(d_livesArrayNext), (void *)&d_livesArrayNext);
	ret = clSetKernelArg(kernel, 2, sizeof(d_rows), (void *)&d_rows);
	ret = clSetKernelArg(kernel, 3, sizeof(d_columns), (void *)&d_columns);
	
	free(source_str);

}

int* init_game_data(){
	N = COLUMNS*ROWS;
	size = sizeof(int)*N;
	// alloc space
	livesArrayActual = (int*) malloc(size);
	// generate first lifes
	int initialAliveCellsSize = (int) COLUMNS*ROWS*0.3;
	int* initialAliveCells = generateInitialLives(1, initialAliveCellsSize);
	for (int i = 0; i < initialAliveCellsSize; i++){
		// initial set up of lives
		livesArrayActual[initialAliveCells[i]] = 1;
	}
	free(initialAliveCells);
	// init resources as needed
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
	// event for blocking 
	cl_event event;
	// time measure
	clock_t start = clock();
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event);
	clWaitForEvents(1, &event); // blocks until kernels finish

	clock_t end = clock();
	double seconds = (float)(end - start) / CLOCKS_PER_SEC;
	return (seconds*1000);
}

int* fetch_gpu_data(){
	ret = clEnqueueReadBuffer(command_queue, d_livesArrayNext, CL_TRUE, 0, size, livesArrayActual, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, d_livesArrayActual, CL_FALSE, 0, size, livesArrayActual, 0, NULL, NULL); // workout for swapping
	return livesArrayActual;
}

/*************************************************************************************************************************************************************************************/
/** SHADER TOOLS **//** SHADER TOOLS **//** SHADER TOOLS **//** SHADER TOOLS **//** SHADER TOOLS **//** SHADER TOOLS **//** SHADER TOOLS **//** SHADER TOOLS **//** SHADER TOOLS **/
/*************************************************************************************************************************************************************************************/

/**
 * Store all the file's contents in memory, useful to pass shaders
 * source code to OpenGL.  Using SDL_RWops for Android asset support.
 */

char* file_read(const char* filename) {
	SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
	if (rw == NULL)
		return NULL;

	Sint64 res_size = SDL_RWsize(rw);
	char* res = (char*) malloc(res_size + 1);

	Sint64 nb_read_total = 0, nb_read = 1;
	char* buf = res;
	while (nb_read_total < res_size && nb_read != 0) {
		nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}
	SDL_RWclose(rw);
	if (nb_read_total != res_size) {
		free(res);
		return NULL;
	}

	res[nb_read_total] = '\0';
	return res;
}

/**
 * Display compilation errors from the OpenGL shader compiler
 */
void print_log(GLuint object) {
	GLint log_length = 0;
	if (glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else if (glIsProgram(object)) {
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else {
		cerr << "printlog: Not a shader or a program" << endl;
		return;
	}

	char* log = (char*) malloc(log_length);

	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);

	cerr << log;
	free(log);
}

/**
 * Compile the shader from file 'filename', with error handling
 */
GLuint create_shader(const char* filename, GLenum type) {
	const GLchar* source = file_read(filename);
	if (source == NULL) {
		cerr << "Error opening " << filename << ": " << SDL_GetError() << endl;
		return 0;
	}
	GLuint res = glCreateShader(type);
	const GLchar* sources[] = {
	source };
	glShaderSource(res, 1, sources, NULL);
	free((void*) source);

	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		cerr << filename << ":";
		print_log(res);
		glDeleteShader(res);
		return 0;
	}

	return res;
}



