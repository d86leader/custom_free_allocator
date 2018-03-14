CXXHEADERS = -std=c++1y -I. -Wall -Wextra -O3
TESTHEADER = -D _CFA_TEST

lib: ;

tests: run-simple_test run-invariant

.PHONY: run-simple_test
run-simple_test: simple_test
	./$<

.PHONY: run-simple_test
run-invariant: invariant
	./$<

simple_test: simple_test.cpp custom_free_allocator.h
	${CXX} -g $< -o $@ $(CXXHEADERS)

invariant: invariant.cpp custom_free_allocator.h
	${CXX} $(TESTHEADER) -g $< -o $@ $(CXXHEADERS)
