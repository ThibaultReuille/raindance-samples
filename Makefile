CC=clang++

# MacOSX CFLAGS
CFLAGS=-W -Wall -ansi -Wno-missing-field-initializers -O2 -std=c++11 -stdlib=libc++ -Wno-deprecated `pkg-config --cflags glew glfw3` -framework OpenCL
LDFLAGS=`pkg-config --static --libs glew glfw3`
#Debug
#CFLAGS=-W -Wall -ansi -Wno-missing-field-initializers -std=c++11 -stdlib=libc++ -Wno-deprecated -framework OpenGL -framework OpenCL -framework GLUT -g

# Linux CFLAGS
#CFLAGS=-W -Wall -ansi -Wno-missing-field-initializers -m64 -O2 -std=c++11 -stdlib=libc++ -Wno-deprecated -lm -lGL -lglut -lOpenCL -g

INCLUDES=-I../ -I../raindance -I./raindance/Lib/ -I../raindance/Lib/glm-0.9.5.4

.PHONY: all prepare

all: prepare window cube particles charts agents fonts stream terminal stereo

prepare:
	( cd ../raindance && make )
	@echo "--- Preparing Build ---"
	mkdir -p Build

window: prepare Window.cc
	@echo "--- Compiling Window Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Window.cc -o Build/window $(LDFLAGS)

cube: prepare Cube.cc
	@echo "--- Compiling Cube Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Cube.cc -o Build/cube $(LDFLAGS)

particles: prepare Particles.cc
	@echo "--- Compiling Particles Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Particles.cc -o Build/particles $(LDFLAGS)

charts: prepare Charts.cc
	@echo "--- Compiling Charts Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Charts.cc -o Build/charts $(LDFLAGS)

agents: prepare Agents.cc
	@echo "--- Compiling Agents Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Agents.cc -o Build/agents $(LDFLAGS)

fonts: prepare Fonts.cc
	@echo "--- Compiling Fonts Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Fonts.cc -o Build/fonts $(LDFLAGS)

stream: prepare Stream.cc
	@echo "--- Compiling Stream Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Stream.cc -o Build/stream $(LDFLAGS)

terminal: prepare Terminal.cc
	@echo "--- Compiling Terminal Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Terminal.cc -o Build/terminal $(LDFLAGS)

stereo: prepare Stereo.cc
	@echo "--- Compiling Stereo Sample ---"
	$(CC) $(CFLAGS) $(INCLUDES) Stereo.cc -o Build/stereo $(LDFLAGS)

clean:
	@echo "--- Cleaning ---"
	rm -rf Build
	rm -rf *~
	rm -rf */*~




