CC = gcc
SHARED_FLAGS = -shared -fPIC -g
SRCS = $(wildcard src/*.c)
HDRS = $(wildcard src/*.h)

gameboy.so: $(SRCS) $(HDRS)
	$(CC) $(SRCS) $(SHARED_FLAGS) -o gameboy.so

lib: gameboy.so
so: gameboy.so

clean:
	rm -f gameboy.so