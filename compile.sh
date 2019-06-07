g++ main.c common.c physics.c loader.c memory.c -o ./sph_alpha -Wall -g -I/usr/include/freetype2 \
	-L/usr/lib/nvidia-346 -L/usr/lib/x86_64-linux-gnu/ -L/usr/local/lib \
	-lglfw -lGLEW -lGLU -lGL -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -lX11 -lXcursor -lfreetype -lSOIL
