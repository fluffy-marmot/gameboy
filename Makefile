.DEFAULT_GOAL := debug

TEST_ROM = tests/testdata/blargg/dmg_sound/rom_singles/01-registers.gb
K =

run:
	python -m clients.pygame.gameboy $(ROM)

testrom:
	python -m clients.pygame.gameboy $(TEST_ROM)

test:
	python -m pytest -k "$(K)"

.PHONY: tests
tests:
	test

testv:
	python -m pytest -v

%:
	$(MAKE) -C core $@