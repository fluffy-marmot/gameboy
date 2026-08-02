.DEFAULT_GOAL := lib

run:
	python -m clients.pygame.gameboy $(ROM)

test:
	python -m pytest 

%:
	$(MAKE) -C core $@