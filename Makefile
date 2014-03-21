CC=clang++

# MacOSX CFLAGS
CFLAGS=-W -Wall -ansi -Wno-missing-field-initializers -m64 -O2 -std=c++11 -stdlib=libc++ -Wno-deprecated -framework OpenGL -framework OpenCL -framework GLUT -g
# Linux CFLAGS
#CFLAGS=-W -Wall -ansi -Wno-missing-field-initializers -m64 -O2 -std=c++11 -stdlib=libc++ -Wno-deprecated -lm -lGL -lglut -lOpenCL -g

INCLUDES=-I../ -I../raindance -I./raindance/Lib/ -I../raindance/Lib/glm-0.9.4.4

prepare:
	( cd ../raindance && make )
	@echo "--- Preparing Build ---"
	mkdir -p Build

cube: prepare
	@echo "--- Compiling Cube Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Cube.cc -o Build/cube

sequences: prepare
	@echo "--- Compiling Sequences Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Sequences.cc -o Build/sequences

particles: prepare
	@echo "--- Compiling Particles Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Particles.cc -o Build/particles

charts: prepare
	@echo "--- Compiling Charts Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Charts.cc -o Build/charts

all: prepare cube sequences particles charts

clean:
	@echo "--- Cleaning ---"
	rm -rf Build
	rm -rf *~
	rm -rf */*~




