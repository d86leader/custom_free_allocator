#include "custom_free_allocator.h"

#include <vector>
#include <iostream>

using std::vector;
using std::cout;
using std::endl;
using std::unique_ptr;

using arr_alloc = custom_free_allocator<int>;

int main()
{
	{
		unique_ptr<int[], arr_alloc::cleanup_deleter> released;
		vector<int, arr_alloc> living_v {4, 5, 6, 7};

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

		auto living_ptr = living_v.get_allocator().own_memory(living_v.data());

		//try the deleter for other thing
		arr_alloc::cleanup_deleter deleter;
		int* test_memory = new int [10];
		deleter(test_memory);
	}

	cout << "no exception means all is good\n";

	return 0;
}
