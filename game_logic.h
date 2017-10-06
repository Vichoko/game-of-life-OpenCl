int* init_game_data(); // se llama al comientzo; devuelve puntero a arreglo de vidas inicial
float kernel_wrapper(); // llamado cada 'frame'; retorna tiempo en segundos 
int* fetch_gpu_data(); // llamado cada 'frame'; retorna puntero a arreglo de vidas actualziado
void free_opencl_resources(); // call at end
