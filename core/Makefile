CC = gcc

CFLAGS = --shared -fPIC -Wmissing-prototypes -g
DEBUG_FLAGS = -Wall -Wextra -Werror

LIB = gameboy.so

SRCS = $(wildcard src/*.c src/mbcs/*.c)
HDRS = $(wildcard src/*.h src/mbcs/*.h)

BUILD_DIR = ../build

all: debug

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/$(LIB): $(SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(SRCS) $(CFLAGS) -o $@

.PHONY: lib
lib: $(BUILD_DIR)/$(LIB)

.PHONY: debug
debug:
	$(MAKE) --always-make lib CFLAGS="$(CFLAGS) $(DEBUG_FLAGS)"

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/$(LIB)