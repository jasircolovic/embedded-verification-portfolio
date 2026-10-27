# Builds the simulated ECU with coverage, runs the pytest integration suite
# (JUnit XML report for CI), then measures which ECU code the tests reached.
CC    := gcc-14
GCOV  := gcov-14
BUILD := build
COV   := -fprofile-arcs -ftest-coverage -fcondition-coverage

.PHONY: test coverage clean

$(BUILD)/motor_ecu: ecu/motor_ecu.c
	mkdir -p $(BUILD)
	$(CC) -std=c99 -Wall -Wextra -Wpedantic -Wconversion -Werror -O0 -g $(COV) -c $< -o $(BUILD)/motor_ecu.o
	$(CC) $(BUILD)/motor_ecu.o --coverage -o $@

test: $(BUILD)/motor_ecu
	rm -f $(BUILD)/*.gcda
	python3 -m pytest tests -q --junitxml=$(BUILD)/integration-report.xml

coverage: test
	$(GCOV) --conditions --branch-probabilities --object-directory $(BUILD) ecu/motor_ecu.c \
	  | grep -E "^(File|Lines|Branches|Condition)"
	mv -f *.gcov $(BUILD)/

clean:
	rm -rf $(BUILD) tests/__pycache__ .pytest_cache
