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
Now you have the memory in `memory` and vector won't delete it! When you want to delete the memory, use
```
destructor(memory);
```
Or if you are happy with smart pointers, you can use some convenience wrappers:
```
auto unique = custom_free_allocator<int>::own_memory(my_vector);
/* above or below, not simultaneously, wouldn't work releasing both */
auto shared = custom_free_allocator<int>::own_shared(my_vector);
```
*Warning*: the vector can still access the memory and place new elements there. It can't delete or destroy them, but vector overwriting elements that should be destroyed is undefined behavior.

## Formal specification

Defined in header `"custom_free_allocator.h"`:
```
template <typename T
         ,typename Delet = std::default_delete<T[]>
         ,typename Alloc = std::allocator<T>
>
class custom_free_allocator;
```
Where:
 - `T` - structure element type
 - `Delet` - deleter used to delete objects not created by allocator
 - `Alloc` - allocator used to allocate/deallocate objects

The main use of `Delet` is for having `unique_ptr` and `shared_ptr` which may at some point own pointer to memory allocated by other means than by `custom_free_allocator`.
### Member types
 - `cleanup_deleter` - deleter used to free memory after it has been acquired from allocator. Can and is also used to free other types of memory.
 - `value_type` - defined as `T`
 - `allocator_type` defined as `Alloc`
 - `deleter_type`   defined as `Delet`
### Member functions
 - `allocate`, `deallocate`, `destroy` - requirements by `allocator_traits`, do some work and pass allocation to `Alloc`.
 - `operator==, operator!=` - all `custom_free_allocator` instances always compare equal.
 - `operator=` - as all `custom_free_allocator` instances are always equal, this will do nothing.

For meaningful methods:
 - `cleanup_deleter release_memory(T* addr);` - make it so the memory at `addr` is not owned by the data structure anymore. Returns a deleter which can free this and any other memory. Throws `std::runtime_error` if `addr` was not allocated by `custom_free_allocator` of appropriate type.
 - `std::unique_ptr<T[], cleanup_deleter> own_memory(T* addr)` - same as `release_memory`, but returns a `unique_ptr` owning the memory with a correct deleter type. Can throw the same `std::runtime_error` if `addr` was not allocated by `custom_free_allocator` of appropriate type.
 - `std::shared_ptr<T> own_shared(T* addr)` - same as `own_memory`, but returns a `std::shared_ptr` owning the memory with a correct deleter type. Can throw the same `std::runtime_error` if `addr` was not allocated by `custom_free_allocator` of appropriate type.
 - `Alloc get_allocator() const;` - returns underlying allocator.

### Static functions
 - `static std::unique_ptr<T[], cleanup_deleter> own_memory(C container);` - same as calling `.own_memory(container.data())`
 - `static std::unique_ptr<T> own_shared(C container);` - same as calling `.own_shared(container.data())`

Allows default, copy and move constructors, which are defined as `= default;`. Requires no destructor.
