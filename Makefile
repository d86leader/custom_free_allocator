CXXHEADERS = -std=c++1y -I. -Wall -Wextra -O3
TESTHEADER = -D _CFA_TEST

lib: ;

tests: run-invariant

.PHONY: run-simple_test
run-invariant: invariant
	./$<

invariant: invariant.cpp custom_free_allocator.h
	${CXX} $(TESTHEADER) -g $< -o $@ $(CXXHEADERS)
