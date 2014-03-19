
CC=clang++
CFLAGS=-W -Wall -ansi -Wno-missing-field-initializers -m64 -O2 -std=c++11 -stdlib=libc++ -Wno-deprecated -g

INCLUDES=-I../ -I../raindance -I./raindance/Lib/ -I../raindance/Lib/glm-0.9.4.4

prepare:
	( cd ../raindance && make )
	@echo "--- Preparing Build ---"
	mkdir -p Build

cube: prepare
	@echo "--- Compiling Cube Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Cube.cc -o Build/cube -framework OpenGL -framework GLUT

sequences: prepare
	@echo "--- Compiling Sequences Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Sequences.cc -o Build/sequences -framework OpenGL -framework GLUT

particles: prepare
	@echo "--- Compiling Particles Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Particles.cc -o Build/particles -framework OpenGL -framework OpenCL -framework GLUT

all: prepare cube sequences particles

clean:
	@echo "--- Cleaning ---"
	rm -rf Build
	rm -rf *~
	rm -rf */*~




