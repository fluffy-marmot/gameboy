.DEFAULT_GOAL := lib

TEST_ROM = tests/testdata/blargg/cpu_instrs/individual/01-special.gb

run:
	python -m clients.pygame.gameboy $(ROM)

testrom:
	python -m clients.pygame.gameboy $(TEST_ROM)

test:
	python -m pytest

%:
	$(MAKE) -C core $@