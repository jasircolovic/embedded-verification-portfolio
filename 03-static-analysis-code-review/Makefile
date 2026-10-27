NAME := test_sensor_filter
SRC  := src_after/sensor_filter.c
TEST := test/test_sensor_filter.c
include ../common.mk
INC  += -I src_after

$(BUILD)/%.o: src_after/%.c | $(BUILD)
	$(CC) $(WARN) $(COV) -O0 -g $(INC) -c $< -o $@

TIDY_CHECKS := -*,clang-analyzer-*,bugprone-*,cert-*
CPPCHECK    := cppcheck --enable=warning,style,portability --addon=misra --std=c99 -q \
               --template='{file}:{line}: [{severity}] {id}: {message}'

# Run all three analyzers on both versions; reports go to reports/.
.PHONY: analyze
analyze:
	mkdir -p reports
	for v in before after; do \
	  gcc-14 -std=c99 -Wall -Wextra -Wconversion -c src_$$v/sensor_filter.c -o /dev/null \
	    > reports/gcc_$$v.txt 2>&1 || true; \
	  $(CPPCHECK) src_$$v/sensor_filter.c > reports/cppcheck_$$v.txt 2>&1 || true; \
	  clang-tidy src_$$v/sensor_filter.c --checks='$(TIDY_CHECKS)' -- -std=c99 \
	    > reports/clang-tidy_$$v.txt 2>/dev/null || true; \
	done
	@for f in reports/*.txt; do printf "%-32s %s findings\n" $$f "$$(grep -cE 'warning|error|\[style\]' $$f)"; done
