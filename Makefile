.DEFAULT_GOAL := debug

TEST_ROM = tests/testdata/mooneye/misc/boot_div-A.gb

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