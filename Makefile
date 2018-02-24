CXXHEADERS = -std=c++1y -I. -Wall -Wextra -O3

lib: ;

tests: simple_test

simple_test: simple_test.cpp custom_free_allocator.h
	${CXX} -g $^ -o $@ $(CXXHEADERS)
