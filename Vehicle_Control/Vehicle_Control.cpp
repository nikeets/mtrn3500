#include <stdio.h>
#include <iostream>

int main()
{
	int i = 0;
	std::cout << "Vehicle Control module" << std::endl;

	while (i < 5000) {
		std::cout << "Vehicle Control module" << std::endl;
		i++;
		System::Threading::Thread::Sleep(10);
	}
	return 0;
}