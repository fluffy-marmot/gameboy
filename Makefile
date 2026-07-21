CC = gcc
SHARED_FLAGS = -shared -fPIC -g

gameboy.so: bus.c bus.h cpu.c cpu.h system.c system.h
	$(CC) bus.c cpu.c system.c $(SHARED_FLAGS) -o gameboy.so

lib: gameboy.so

clean:
	rm -f gameboy.so