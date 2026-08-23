CC = gcc
EMCC = emcc
TSC := $(shell command -v tsc 2>/dev/null)

SRCS = $(wildcard src/*.c src/mbcs/*.c)
HDRS = $(wildcard src/*.h src/mbcs/*.h)

BUILD_DIR = ../build
BUILD_DIR_WEB = $(BUILD_DIR)/web

############### for local .so build ###############

CFLAGS = --shared -fPIC  -fvisibility=hidden
DEBUG_FLAGS = -g -Wmissing-prototypes -Wall -Wextra -Werror

LIB = gameboy.so

all: lib web

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/$(LIB): $(SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(SRCS) $(CFLAGS) -o $@

.PHONY: lib
lib: $(BUILD_DIR)/$(LIB)

.PHONY: debug
debug:
	$(MAKE) --always-make lib CFLAGS="$(CFLAGS) $(DEBUG_FLAGS)"

############### for emscripten web build ###############

# add  -Wno-c23-extensions to silence warnings? or edit anything?
EMCC_FLAGS = -sMODULARIZE=1 -sEXPORT_ES6=1  										\
			 -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8","HEAPF32"]' 	\
			 -s EXPORTED_FUNCTIONS='["_malloc","_free"]'

# check whether typescript is available, it's not a hard build dependency but
# if it is, add flag to create a typescript signature file for better .js typehints
ifdef TSC
EMCC_FLAGS += --emit-tsd gameboy.d.mts
endif
LIB_WEB = gameboy.mjs

$(BUILD_DIR_WEB):
	mkdir -p $@

$(BUILD_DIR_WEB)/$(LIB_WEB): $(SRCS) $(HDRS) | $(BUILD_DIR_WEB)
	$(EMCC) $(SRCS) $(EMCC_FLAGS) -o $@

.PHONY: web
web: $(BUILD_DIR_WEB)/$(LIB_WEB)

############### misc. ###############

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/$(LIB) $(BUILD_DIR_WEB)