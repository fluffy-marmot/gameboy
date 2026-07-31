.DEFAULT_GOAL := lib

# default to this test ROM
ROM_PATH = romlib
TEST_PATH = tests/testdata

run:
	python -m clients.pygame.gameboy $(ROM_PATH)/$(ROM)

test:
	python -m clients.pygame.gameboy $(TEST_PATH)/$(ROM)

%:
	$(MAKE) -C core $@