.DEFAULT_GOAL := debug

TEST_ROM = tests/testdata/blargg/instr_timing/instr_timing.gb
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