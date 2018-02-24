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
	unique_ptr<int, arr_alloc::cleanup_deleter> released;

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
			cout << "first element of released: " << *released <<endl;
		}
	}
	if (released)
	{
		cout << "released memory: ";
		for (int i = 0; i < 5; ++i)
		{
			cout << released.get()[i] << ' ';
		}
		cout << endl;
	}

	//try the deleter for other thing
	arr_alloc::cleanup_deleter deleter;
	int* test_memory = new int [10];
	deleter(test_memory);
	cout << "no exception means al is good\n";

	return 0;
}
