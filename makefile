CPP = g++
CPPFLAGS = -I/usr/include/SDL2 -g
LDLIBS = -lSDL2 -lGLEW -lGL

CLFLAGS = -std=c++0x -lOpenCL -I/usr/local/cuda-8.0/include

#main : game_logic.o controller.o
#	$(CPP) -o main game_logic.o controller.o $(LDLIBS)

#controller.o : controller.cpp
#	$(CPP) $(CPPFLAGS) -c controller.cpp $(LDLIBS)

#game_logic.o : game_logic.h
#	$(CPP) $(CLFLAGS) game_logic.cpp 

main: controller.cpp
	g++ -I/usr/include/SDL2 -g -o main controller.cpp -lSDL2 -lGLEW -lGL -std=c++0x -lOpenCL -I/usr/local/cuda-8.0/include

clean :
	rm -f *.o main



