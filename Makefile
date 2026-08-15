.DEFAULT_GOAL := debug

TEST_ROM = tests/testdata/dmg_mode1_stat_int/dmg_mode1_stat_int_tests.gb
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