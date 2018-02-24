#include <memory>
#include <set>
#include <stdexcept>

template <typename T
         ,typename Delet = std::default_delete<T>
         ,typename Alloc = std::allocator<T>
         >
class custom_free_allocator
{
private:
	static std::set<std::pair<T*, size_t>> used_memory;
	static std::set<std::pair<T*, size_t>> moved_memory;

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
				if (it->first == p)
				{
					Alloc allocator;
					allocator.deallocate(p, it->second);
					moved_memory.erase(it);
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
		auto memory = m_allocator.allocate(n);
		used_memory.emplace(memory, n);
		return memory;
	}

	void deallocate(T* p, size_t n)
	{
		// testing if was moved
		for (auto it = moved_memory.begin(); it != moved_memory.end(); ++it)
		{
			if (it->first == p)
			{
				//not deallocating any objects\n
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
			if (mem.first <= p && mem.first + mem.second > p)
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
			if (t.first == addr)
			{
				//releasing memory
				moved_memory.emplace(t.first, t.second);
				return cleanup_deleter();
			}
		}
		throw std::runtime_error("releasing unacquired memory\n");
	}

	// make memory not owned by structure anymore
	// returns unique_ptr owning the memory that will delete it correctly
	std::unique_ptr<T, cleanup_deleter> own_memory(T* addr)
	{
		for (auto t : used_memory)
		{
			if (t.first == addr)
			{
				//releasing memory
				moved_memory.emplace(t.first, t.second);
				return std::unique_ptr<T, cleanup_deleter>(addr);
			}
		}
		throw std::runtime_error("releasing unaquired memory");
	}

	// make memory not owned by structure anymore
	// returns shared_ptr owning the memory that will delete it correctly
	std::shared_ptr<T> own_shared(T* addr)
	{
		for (auto t : used_memory)
		{
			if (t.first == addr)
			{
				//releasing memory
				moved_memory.emplace(t.first, t.second);
				return std::shared_ptr<T>(addr, cleanup_deleter());
			}
		}
		throw std::runtime_error("releasing unaquired memory");
	}

	Alloc get_allocator() const {return m_allocator;}

	friend bool operator== (
		const custom_free_allocator<T, Alloc, Delet>& l,
		const custom_free_allocator<T, Alloc, Delet>& r)
	{
		return true;
	}
	friend bool operator!= (
		const custom_free_allocator<T, Alloc, Delet>& l,
		const custom_free_allocator<T, Alloc, Delet>& r)
	{
		return false;
	}
};

// initialize static variables

template <typename T, typename A, typename D>
std::set<std::pair<T*, size_t>>
	custom_free_allocator<T, A, D>::moved_memory =
		std::set<std::pair<T*, size_t>>();

template <typename T, typename A, typename D>
std::set<std::pair<T*, size_t>>
	custom_free_allocator<T, A, D>::used_memory =
		std::set<std::pair<T*, size_t>>();
