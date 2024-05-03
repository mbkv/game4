CXX = clang++

CXXFLAGS = -std=c++2b \
         -I$(CURDIR) \
		 -I$(CURDIR)/vendor \
         -Wall \
         -Wno-unused-parameter \
         -Wno-unused-function \
		 -Wno-unknown-pragmas \
         -DTINYOBJ_LOADER_C_IMPLEMENTATION \
         -DSTB_IMAGE_IMPLEMENTATION \
         -DFAST_OBJ_IMPLEMENTATION \
         -msse4.2 \
         -ffast-math \
         -fassociative-math \
         -fno-exceptions \
         -fno-unwind-tables \
         -fno-asynchronous-unwind-tables \
         -fvisibility=hidden \
         -DSLOW

CXXFLAGS_DEBUG = -O0 \
               -g3 \
               -DDEBUG

EMCC_FLAGS = --use-port=sdl2 \
			 -s USE_WEBGL2=1 \
			 -mrelaxed-simd \
			 -sFETCH \
			 -gsource-map \
			 -sSTACK_SIZE=1048576 \
			 -sALLOW_MEMORY_GROWTH \
			 -sENVIRONMENT=web \
			 -sMAX_WEBGL_VERSION=2 \
			 -sMIN_WEBGL_VERSION=2 \
			 -sMALLOC=emmalloc-debug \
			 -sUSE_CLOSURE_COMPILER=1 \
			 -sGL_ASSERTIONS=1 \
			 -sGL_ENABLE_GET_PROC_ADDRESS=0

include_files := $(wildcard src/*.hpp)

.PHONY: dev
dev: build/game_debug.html | build build/res build/src build/vendor

.PHONY: watch
watch: dev
	while true; do \
		inotifywait -qr -e modify -e create -e delete -e move src; \
		make dev; \
	done

build: |
	mkdir -p build

build/game_debug.html: src/main.cpp $(include_files) | build
	emcc $(CXXFLAGS) $(CXXFLAGS_DEBUG) $(EMCC_FLAGS) $< -o $@

build/game_debug: src/main.cpp $(include_files) | build
	#$(CXX) $(CXXFLAGS) -march=native $(CXXFLAGS_DEBUG) $< -o $@

build/res: | build
	ln -s $(CURDIR)/res $(CURDIR)/build/res

build/src: | build
	ln -s $(CURDIR)/src $(CURDIR)/build/src

build/vendor: | build
	ln -s $(CURDIR)/vendor $(CURDIR)/build/vendor

clean:
	rm -rf build
