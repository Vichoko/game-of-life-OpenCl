CPP = g++
CPPFLAGS = -I/usr/include/SDL2 -I/usr/local/cuda-8.0/include -g
LDLIBS = -lSDL2 -lGLEW -lGL -std=c++0x -lOpenCL


opencl: controller.cpp
	$(CPP) $(CPPFLAGS) -o opencl controller.cpp  $(LDLIBS)

clean :
	rm -f *.o opencl



