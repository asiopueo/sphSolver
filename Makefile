#
CC = g++
LD = ld

DEBUG = -g

CFLAGS = -std=gnu++11 -Wall -c 

LDFLAGS = -L/usr/lib/nvidia-346 -L/usr/local/lib/x86_64-linux-gnu -L/usr/local/lib \
				-lfreetype -lglfw -lGLEW -lGLU -lGL -lXxf86vm -lXrandr -lXi -lXinerama -lX11 -lXcursor -lSOIL -lpthread -lm

#OBJS = src/main.o src/loader.o src/triangle.o src/skybox.o src/camera.o src/window.o src/freetype.o src/ObjLoader.o


all: main.o window.o camera.o skybox.o marchingcubes.o
	g++ build/*.o $(LDFLAGS) -o mc_test

main.o: src/main.cpp
	$(CC) $(CFLAGS) -I/usr/include/freetype2 src/main.cpp -o build/main.o

window.o: src/window.cpp
	$(CC) $(CFLAGS) src/window.cpp -o build/window.o

camera.o: src/camera.cpp
	$(CC) $(CFLAGS) src/camera.cpp -o build/camera.o

skybox.o: src/skybox.cpp
	$(CC) $(CFLAGS) src/skybox.cpp -o build/skybox.o

marchingcubes.o: src/marchingcubes.cpp
	$(CC) $(CFLAGS) src/marchingcubes.cpp -o build/marchingcubes.o


clean:
	rm ./build/*.o

