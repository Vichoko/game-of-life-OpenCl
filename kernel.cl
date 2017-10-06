/**__kernel void hello(__global char* string)
{
string[0] = 'H';
string[1] = 'e';
string[2] = 'l';
string[3] = 'l';
string[4] = 'o';
string[5] = ',';
string[6] = ' ';
string[7] = 'W';
string[8] = 'o';
string[9] = 'r';
string[10] = 'l';
string[11] = 'd';
string[12] = '!';
string[13] = '\0';
}

void kernel simple_add(global const int* A, global const int* B, global int* C){                    
	C[get_global_id(0)] = A[get_global_id(0)] + B[get_global_id(0)];

} 

bool isCellAlive(int* array, int column, int row){
	return array[row*COLUMNS + column];

}
void setCellAlive(int* array, int column, int row){
	array[row*COLUMNS + column] = 1;
}
void setCellDead(int* array, int column, int row){
	array[row*COLUMNS + column] = 0;
}
int countAliveNeighbors(int* livesArray, int column, int row){
	int neighborColumn;
	int neighborRow;
	int aliveNeighbors = 0;
	for (int x = -1; x <= 1; x++){
		for (int y = -1; y <= 1; y++){
			if (x == 0 && y == 0) // itself bypass
				continue;
			neighborColumn = column+x;
			neighborRow = row+y;

			// edge conditions
			if (neighborColumn < 0){
				neighborColumn = COLUMNS-1;
			} else if (neighborColumn > COLUMNS-1){
				neighborColumn = 0;
			} if (neighborRow < 0){
				neighborRow = ROWS-1;
			} else if (neighborRow > ROWS-1){
				neighborRow = 0;
			}

			if (isCellAlive(livesArray, neighborColumn, neighborRow)){
				aliveNeighbors++;
			}
		}
	}
	return aliveNeighbors;
}
	*/


__kernel void refresh_life(__global int* livesArrayActual, __global int* livesArrayNext){                    
	int index = get_global_id(0);
	int COLUMNS = 64;
	int ROWS = 64;
	
	if (get_global_id(0) < COLUMNS*ROWS){
		
		int row = (int)(index / COLUMNS);
		int column = index - row*COLUMNS;
		
		// COUNT ALIVE NEIGHBORS ROUTINE
		int aliveNeighbors = 0;

		int neighborColumn;
		int neighborRow;
		for (int x = -1; x <= 1; x++){
			for (int y = -1; y <= 1; y++){
				if (x == 0 && y == 0) // itself bypass
					continue;
				neighborColumn = column+x;
				neighborRow = row+y;

				// edge conditions
				if (neighborColumn < 0){
					neighborColumn = COLUMNS-1;
				} else if (neighborColumn > COLUMNS-1){
					neighborColumn = 0;
				} if (neighborRow < 0){
					neighborRow = ROWS-1;
				} else if (neighborRow > ROWS-1){
					neighborRow = 0;
				}
				if (livesArrayActual[neighborRow*COLUMNS + neighborColumn]){
					aliveNeighbors++;
				}
			}
		}
		if ((livesArrayActual[row*COLUMNS + column] && (aliveNeighbors == 2 || aliveNeighbors == 3)) ||
				(!livesArrayActual[row*COLUMNS + column] && (aliveNeighbors == 3 || aliveNeighbors == 6))){
			// life condition
			livesArrayNext[row*COLUMNS + column] = 1;

		} else {
			// death condition
			livesArrayNext[row*COLUMNS + column] = 0;
		}

	} 
	
} 
