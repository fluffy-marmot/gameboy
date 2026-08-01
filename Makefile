.DEFAULT_GOAL := lib

# default to this test ROM
TEST_PATH = tests/testdata

run:
	python -m clients.pygame.gameboy $(ROM)

test:
	python -m clients.pygame.gameboy $(TEST_PATH)/$(ROM)

%:
	$(MAKE) -C core $@