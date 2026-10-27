NAME := test_door_interlock
SRC  := src/door_interlock.c
TEST := test/test_door_interlock.c
include ../common.mk

# Same tests minus V3/V4: statement and branch coverage stay high, MC/DC drops.
.PHONY: coverage-partial
coverage-partial:
	$(MAKE) clean
	$(MAKE) coverage EXTRA=-DPARTIAL_SUITE
	grep -B1 -A3 "not covered" $(BUILD)/door_interlock.c.gcov || true
