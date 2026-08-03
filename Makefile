.DEFAULT_GOAL := debug

TEST_ROM = tests/testdata/mealybug/m2_win_en_toggle.gb

run:
	python -m clients.pygame.gameboy $(ROM)

testrom:
	python -m clients.pygame.gameboy $(TEST_ROM)

test:
	python -m pytest

.PHONY: tests
tests:
	test

testv:
	python -m pytest -v

%:
	$(MAKE) -C core $@