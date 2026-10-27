# Shared build rules. Each project sets SRC, TEST and NAME, then includes this file.
CC      := gcc-14
GCOV    := gcov-14
UNITY   := ../unity
WARN    := -std=c99 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror
COV     := -fprofile-arcs -ftest-coverage -fcondition-coverage
INC     := -I src -I test -I $(UNITY)
BUILD   := build

.PHONY: test coverage clean

$(BUILD)/unity.o: $(UNITY)/unity.c | $(BUILD)
	$(CC) -std=c99 -O0 -g -I $(UNITY) -c $< -o $@

# Code under test is instrumented for coverage; test code and Unity are not.
OBJ := $(addprefix $(BUILD)/,$(notdir $(SRC:.c=.o)))

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(WARN) $(COV) -O0 -g $(INC) -c $< -o $@

$(BUILD)/$(NAME): $(OBJ) $(TEST) $(BUILD)/unity.o | $(BUILD)
	$(CC) $(WARN) $(EXTRA) -O0 -g $(INC) $(TEST) $(OBJ) $(BUILD)/unity.o --coverage -o $@

$(BUILD):
	mkdir -p $(BUILD)

test: $(BUILD)/$(NAME)
	rm -f $(BUILD)/*.gcda
	./$(BUILD)/$(NAME)

# Statement, branch and MC/DC (condition) coverage of the code under test only.
coverage: test
	$(GCOV) --conditions --branch-probabilities --object-directory $(BUILD) $(SRC) \
	  | grep -E "^(File|Lines|Branches|Condition)"
	mv -f *.gcov $(BUILD)/
	@echo "Annotated sources: $(BUILD)/*.c.gcov"

clean:
	rm -rf $(BUILD)
