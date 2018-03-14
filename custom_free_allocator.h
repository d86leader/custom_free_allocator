// copyright: d86leader, 2018
// created and maintained by d86leader@github.com
// released under BSD licence
#include <memory>
#include <list>
#include <stdexcept>

// used to indicate what memory has been allocated
// previously an std::pair, but having a custom struct is more readable
template<typename T>
struct _CFA_used_mem_status
{
	_CFA_used_mem_status(T* a, size_t s) : address(a), size(s) {}
	T* address;
	size_t size;
};
// used to indicate how many entities still use the moved memory
template<typename T>
struct _CFA_moved_mem_status
{
	//2 means it is used by vector and deleter, which is always a starting
	//point
	_CFA_moved_mem_status(T* a, size_t s) : address(a), size(s), use_count(2) {}
	T* address;
	size_t size;
	int use_count; //should be 1, 2 or 0
};

template <typename T
         ,typename Delet = std::default_delete<T[]>
         ,typename Alloc = std::allocator<T>
         >
class custom_free_allocator
{
	#ifdef _CFA_TEST
	  friend int main();
	#endif //def _CFA_TEST
private:
	static std::list<_CFA_used_mem_status<T>>  used_memory;
	static std::list<_CFA_moved_mem_status<T>> moved_memory;

	// Used when an entity stops using released memory.
	// Decreases use_count and frees the memory when nothing uses it.
	// A template is used here as there were some problems with instancing
	// a set<T>::iterator type
	template <typename iterator>
	static void decrease_use_count(iterator it)
	{
		it->use_count -= 1;
		if (it->use_count == 0)
		{
			Alloc allocator;
			allocator.deallocate(it->address, it->size);
			moved_memory.erase(it);
			return;
		}
	}

	Alloc m_allocator;
public:
	// A helper struct used to deallocate released memory.
	// Best used with smart pointers
	struct cleanup_deleter
	{
		// Delete released memory.
		// As a smart pointer can sometimes have some other kind of memory,
		// it will also free a memory if it was not found in moved index
		void operator() (T* p)
		{
			// testing if was moved
			for (auto it = moved_memory.begin();
			     it != moved_memory.end();
			     ++it)
			{
				if (it->address == p)
				{
					custom_free_allocator<T, Delet, Alloc> ::
						decrease_use_count(it);
					return;
				}
			}
			// not found in moved memory: simply delete
			Delet d;
			d(p);
		}
	};
	friend cleanup_deleter;

	using value_type     = T;
	using allocator_type = Alloc;
	using deleter_type   = Delet;

	custom_free_allocator()                             = default;
	custom_free_allocator(const custom_free_allocator&) = default;
	custom_free_allocator(custom_free_allocator&&)      = default;

	custom_free_allocator& operator= (const custom_free_allocator&)
	{
		return *this;
	}

	// Allocator requirement
	T* allocate(size_t n)
	{
		//allocating n objects
		auto address = m_allocator.allocate(n);
		used_memory.emplace_back(address, n);
		return address;
	}

	// Allocator requirement
	void deallocate(T* p, size_t n)
	{
		// testing if was moved
		for (auto it = moved_memory.begin(); it != moved_memory.end(); ++it)
		{
			if (it->address == p)
			{
				custom_free_allocator<T, Delet, Alloc>::decrease_use_count(it);
				return;
			}
		}
		//deallocating n objects
		m_allocator.deallocate(p, n);
	}

	// Allocator requirement
	template< class U >
	void destroy( U* p )
	{
		// removing only if it was not in any moved memory
		for (auto mem : moved_memory)
		{
			if (mem.address <= p && mem.address + mem.size > p)
			{
				//not destroying any object
				return;
			}
		}
		//destroying an object
		m_allocator.destroy(p);
	}

	// Make memory not owned by structure anymore.
	// Returns the deleter for this memory.
	cleanup_deleter release_memory(T* addr)
	{
		// find the memory in used index
		for (auto it = used_memory.begin(); it != used_memory.end(); ++it)
		{
			if (it->address == addr)
			{
				//releasing memory
				moved_memory.emplace_back(it->address, it->size);
				used_memory.erase(it);
				return cleanup_deleter();
			}
		}
		throw std::runtime_error("releasing unacquired memory\n");
	}

	// make memory not owned by structure anymore
	// returns unique_ptr owning the memory that will delete it correctly
	std::unique_ptr<T[], cleanup_deleter> own_memory(T* addr)
	{
		this->release_memory(addr);
		return std::unique_ptr<T[], cleanup_deleter>(addr);
	}

	// make memory not owned by structure anymore
	// returns shared_ptr owning the memory that will delete it correctly
	std::shared_ptr<T> own_shared(T* addr)
	{
		this->release_memory(addr);
		return std::shared_ptr<T>(addr, cleanup_deleter());
	}

	// static wrappers for two methods above
	template<typename C>
	static inline
	std::unique_ptr<T[], cleanup_deleter> own_memory(C container)
	{
		return container.get_allocator().own_memory(container.data());
	}
	template<typename C>
	static inline
	std::shared_ptr<T> own_shared(C container)
	{
		return container.get_allocator().own_shared(container.data());
	}


	Alloc get_allocator() const {return m_allocator;}

	friend bool operator== (
		const custom_free_allocator<T, Delet, Alloc>&,
		const custom_free_allocator<T, Delet, Alloc>&)
	{
		return true;
	}
	friend bool operator!= (
		const custom_free_allocator<T, Delet, Alloc>&,
		const custom_free_allocator<T, Delet, Alloc>&)
	{
		return false;
	}
};

// initialize static variables

template <typename T, typename D, typename A>
std::list<_CFA_moved_mem_status<T>>
	custom_free_allocator<T, D, A>::moved_memory =
		std::list<_CFA_moved_mem_status<T>>();

template <typename T, typename D, typename A>
std::list<_CFA_used_mem_status<T>>
	custom_free_allocator<T, D, A>::used_memory =
		std::list<_CFA_used_mem_status<T>>();
