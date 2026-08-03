.DEFAULT_GOAL := lib

TEST_ROM = tests/testdata/mealybug/m2_win_en_toggle.gb

run:
	python -m clients.pygame.gameboy $(ROM)

testrom:
	python -m clients.pygame.gameboy $(TEST_ROM)

test:
	python -m pytest

testv:
	python -m pytest -v

%:
	$(MAKE) -C core $@