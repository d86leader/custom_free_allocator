# Custom-free allocator

A C++ library that allows you to take control of the memory used by standart data structures.

## Ideas

Have you ever thought of simply taking the vector's memory for yourself? Taking control of the memory `std::set` uses? Then this library is just for you!
It allows you to do just that, and also provides interfaces to own the acquired memory by smart pointer!

## Usage

You only need to include `custom_free_allocator.h` in your project for it to work.

Once you have it included, create a vector or other data structure using the custom allocator provided:
```
std::vector< int, custom_free_allocator<int> > my_vector;
```
Then, once your vector has some elements you want to take control of, take the following steps:
1. Get the custom allocator of your vector
2. Get the address of the memory region owned by vector
3. Release the address from the vector
```
auto alloc = my_vector.get_allocator();
int* memory = my_vector.data();
auto destructor = alloc.release_memory(memory);
```
Now you have the memory in `memory` and vector won't delete it! But the vector can still access it and place new elements there. When you want to delete the memory, use
```
destructor(memory);
```
