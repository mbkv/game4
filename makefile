CC = emcc
CFLAGS = -std=c++2b \
         -I$(CURDIR) \
         -Wall \
         -Wextra \
         -Wno-unused-parameter \
         -Wno-unused-function \
         -Wno-missing-field-initializers \
         -DTINYOBJ_LOADER_C_IMPLEMENTATION \
         -DSTB_IMAGE_IMPLEMENTATION \
         -DFAST_OBJ_IMPLEMENTATION \
         -msse4.2 \
         -ffast-math \
         -fassociative-math \
         -fno-exceptions \
         -fno-unwind-tables \
         -fno-asynchronous-unwind-tables \
         -DSLOW

CFLAGS_DEBUG = -O0 \
               -g3 \
               -DDEBUG

EMCC_FLAGS = --use-port=sdl2 -s USE_WEBGL2=1 -mrelaxed-simd -sFETCH -gsource-map

include_files := $(wildcard src/*.hpp)

.PHONY: dev
dev: build/game_debug.html build/res

build:
	mkdir -p build

build/game_debug.html: src/main.cpp $(include_files) build
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) $(EMCC_FLAGS) $< -o $@

build/res: build
	ln -s $(CURDIR)/res $(CURDIR)/build/res

build/src: build
	ln -s $(CURDIR)/src $(CURDIR)/build/src

build/vendor: build
	ln -s $(CURDIR)/vendor $(CURDIR)/build/vendor

clean:
	rm -rf build
