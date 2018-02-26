CXXHEADERS = -std=c++1y -I. -Wall -Wextra -O3

lib: ;

tests: run-simple_test

.PHONY: run-simple_test
run-simple_test: simple_test
	./simple_test

simple_test: simple_test.cpp custom_free_allocator.h
	${CXX} -g $^ -o $@ $(CXXHEADERS)
