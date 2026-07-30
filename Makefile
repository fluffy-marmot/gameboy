.DEFAULT_GOAL := lib

# default to this test ROM
ROM = prehistorik

run:
	python -m clients.pygame.gameboy $(ROM)

%:
	$(MAKE) -C core $@