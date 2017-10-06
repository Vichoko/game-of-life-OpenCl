using namespace std;
typedef struct {
	float* array;
	int total_len;
	int vertex_len;
	} container_t;


container_t* lives_array_to_bw_squares_vertices();
bool init_resources(void);


/**
 *
 * Calcula nuevos vertices y colores y los pasa a la gpu, luego dibuja.*/
void render(SDL_Window* window);
/**
 *
 *
 * Loop del juego de la vida.
 * */
void mainLoop(SDL_Window* window);
/**
 * Inicia ventana y recursos.
 * Retorna referencia a ventana.
 *
 * */
SDL_Window* init_display_stuff();
void free_resources();
void intHandler(int dummy);
