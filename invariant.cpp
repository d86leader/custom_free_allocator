
#include "custom_free_allocator.h"

#include <vector>
#include <iostream>

using std::vector;
using std::cout;
using std::endl;
using std::unique_ptr;
using std::list;

using arr_alloc = custom_free_allocator<int>;

int main()
{
	list<_CFA_used_mem_status<int>>& used_mem = arr_alloc::used_memory;
	list<_CFA_moved_mem_status<int>>& moved_mem = arr_alloc::moved_memory;
	if (used_mem.size() != 0)
	{
		throw std::runtime_error("used_mem not zero when it should!");
	}
	if (moved_mem.size() != 0)
	{
		throw std::runtime_error("moved_mem not zero when it should!");
	}

	{
		unique_ptr<int[], arr_alloc::cleanup_deleter> released;
		vector<int, arr_alloc> living_v {4, 5, 6, 7};
		vector<int, arr_alloc> another_v {10, 11, 12, 13};

		{
			vector<int, arr_alloc> v;

			v.push_back(1);
			v.push_back(2);
			v.push_back(3);
			v.push_back(4);
			v.push_back(5);

			released = v.get_allocator().own_memory(v.data());
			if (released == nullptr)
			{
				cout << "did not release memory properly\n";
			}
			else
			{
				cout << "first element of released: " << released[0] <<endl;
			}

			auto living_ptr = living_v.get_allocator().own_memory(living_v.data());
			cout << "removing living_ptr\n";
		}
		cout << "done\n";
		if (released)
		{
			cout << "released memory: ";
			for (int i = 0; i < 5; ++i)
			{
				cout << released.get()[i] << ' ';
			}
			cout << endl;
		}

		auto other_ptr = custom_free_allocator<int>::own_shared(another_v);
		cout << "tried to get a living_ptr\n";

		//try the deleter for other thing
		arr_alloc::cleanup_deleter deleter;
		int* test_memory = new int [10];
		deleter(test_memory);
		cout << "tried the deleter for other thing\n";
	}

	if (used_mem.size() != 0)
	{
		throw std::runtime_error("used_mem not zero when it should!");
	}
	if (moved_mem.size() != 0)
	{
		throw std::runtime_error("moved_mem not zero when it should!");
	}

	cout << "no exception means all is good\n";

	return 0;
}
