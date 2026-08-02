.DEFAULT_GOAL := lib

# default to this test ROM
TEST_PATH = tests/testdata

run:
	python -m clients.pygame.gameboy $(ROM)

test:
	python -m pytest 

%:
	$(MAKE) -C core $@