PROJECTS := 01-requirements-based-testing 02-mcdc-coverage 03-static-analysis-code-review \
            04-hal-mocking-fault-injection 05-integration-tests-python

.PHONY: all clean $(PROJECTS)

# Clean build, unit + integration tests, coverage for every project, static analysis.
all: clean $(PROJECTS)
	$(MAKE) -C 03-static-analysis-code-review analyze
	$(MAKE) -C 02-mcdc-coverage coverage-partial
	@echo "ALL PROJECTS PASSED"

$(PROJECTS):
	$(MAKE) -C $@ coverage

clean:
	for p in $(PROJECTS); do $(MAKE) -C $$p clean; done
