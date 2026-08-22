SRCS = $(wildcard src/*.c src/mbcs/*.c)
HDRS = $(wildcard src/*.h src/mbcs/*.h)

############### for local .so build ###############
CC = gcc

CFLAGS = --shared -fPIC  -fvisibility=hidden
DEBUG_FLAGS = -g -Wmissing-prototypes -Wall -Wextra -Werror

LIB = gameboy.so

BUILD_DIR = ../build

all: debug web

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
EMCC = emcc

# add  -Wno-c23-extensions to silence warnings? or edit anything?
EMCC_FLAGS = -sMODULARIZE=1 -sEXPORT_ES6=1 -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'
LIB_WEB = gameboy.mjs

BUILD_DIR_WEB = ../build/web

$(BUILD_DIR_WEB)/$(LIB_WEB): $(SRCS) $(HDRS) | $(BUILD_DIR_WEB)
	$(EMCC) $(SRCS) $(EMCC_FLAGS) -o $@

$(BUILD_DIR_WEB):
	mkdir -p $@

.PHONY: web
web: $(BUILD_DIR_WEB)/$(LIB_WEB)


############### misc. ###############

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/$(LIB)