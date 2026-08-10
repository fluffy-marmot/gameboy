.DEFAULT_GOAL := debug

TEST_ROM = tests/testdata/blargg/oam_bug/rom_singles/7-timing_effect.gb
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