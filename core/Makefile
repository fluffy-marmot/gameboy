CC = gcc
SRCS = $(wildcard src/*.c src/mbcs/*.c)
HDRS = $(wildcard src/*.h src/mbcs/*.h)
BUILD_DIR = ../build
# -g is same as --debug
FLAGS = --shared -fPIC -Wmissing-prototypes -g
EXTRA_WARNING_FLAGS = -Wall -Wextra
.DEFAULT_GOAL := lib

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/gameboy.so: $(SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(SRCS) $(FLAGS) -o $(BUILD_DIR)/gameboy.so

.PHONY: gameboy.so
gameboy.so: $(BUILD_DIR)/gameboy.so

.PHONY: lib
lib: $(BUILD_DIR)/gameboy.so

.PHONY: warn
warn:
	$(MAKE) --always-make lib FLAGS="$(FLAGS) $(EXTRA_WARNING_FLAGS)"

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/gameboy.so