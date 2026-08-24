TEST_ROM = tests/testdata/blargg/instr_timing/instr_timing.gb
TEST_PORT = 8000
# K used to specify a subset of test suite for convenience, e.g. "mooneye"
K =

run:
	python -m clients.pygame.gameboy $(ROM)

testrom:
	python -m clients.pygame.gameboy $(TEST_ROM)

test:
	python -m pytest -k "$(K)"

testv:
	python -m pytest -v

testserver:
	@echo "Test Web Client: http://localhost:$(TEST_PORT)/clients/web/index.html"
	python -m http.server $(TEST_PORT)

%:
	$(MAKE) -C core $@
