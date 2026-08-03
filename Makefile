.DEFAULT_GOAL := lib

TEST_ROM = tests/testdata/blargg/oam_bug/oam_bug.gb

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