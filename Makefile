.DEFAULT_GOAL := lib

run:
	python -m clients.pygame.gameboy

%:
	$(MAKE) -C core $@