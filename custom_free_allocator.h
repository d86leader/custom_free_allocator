// copyright: d86leader, 2018
// created and maintained by d86leader@github.com
// released under BSD licence
#include <memory>
#include <list>
#include <stdexcept>

template<typename T>
struct _CFA_used_mem_status
{
	_CFA_used_mem_status(T* a, size_t s) : address(a), size(s) {}
	T* address;
	size_t size;
};
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
private:
	static std::list<_CFA_used_mem_status<T>>  used_memory;
	static std::list<_CFA_moved_mem_status<T>> moved_memory;

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
	struct cleanup_deleter
	{
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

	T* allocate(size_t n)
	{
		//allocating n objects
		auto address = m_allocator.allocate(n);
		used_memory.emplace_back(address, n);
		return address;
	}

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

	// make memory not owned by structure anymore
	// returns the deleter for this memory
	cleanup_deleter release_memory(T* addr)
	{
		for (auto t : used_memory)
		{
			if (t.address == addr)
			{
				//releasing memory
				moved_memory.emplace_back(t.address, t.size);
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
	std::shared_ptr<T[]> own_shared(T* addr)
	{
		this->release_memory(addr);
		return std::shared_ptr<T[]>(addr, cleanup_deleter());
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
