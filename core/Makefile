CC = gcc
SRCS = $(wildcard src/*.c src/mbcs/*.c)
HDRS = $(wildcard src/*.h src/mbcs/*.h)
BUILD_DIR = ../build
# -g is same as --debug
SHARED_FLAGS = --shared -fPIC
WARNING_FLAGS = -Wmissing-prototypes -g
.DEFAULT_GOAL := lib

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/gameboy.so: $(SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(SRCS) $(SHARED_FLAGS) $(WARNING_FLAGS) -o $(BUILD_DIR)/gameboy.so

.PHONY: gameboy.so
gameboy.so: $(BUILD_DIR)/gameboy.so
.PHONY: lib
lib: gameboy.so

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/gameboy.so