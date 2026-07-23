CC = gcc
SHARED_FLAGS = -shared -fPIC -g
SRCS = $(wildcard src/*.c)

gameboy.so: $(SRCS)
	$(CC) $(SRCS) $(SHARED_FLAGS) -o gameboy.so

lib: gameboy.so

clean:
	rm -f gameboy.so