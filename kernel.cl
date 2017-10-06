__kernel void refresh_life(__global int* livesArrayActual, __global int* livesArrayNext, __global int* rows,  __global int* columns){                    
	int index = get_global_id(0);
	int COLUMNS = *columns;
	int ROWS = *rows;
	
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
