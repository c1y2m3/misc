#include <iostream>

#include "WinAPIHelper.h"

int main() {
	std::vector<Application*> applications = WinAPIHelper::getApplicationList();

	std::cout << "Found " << applications.size() << " applications: \n";
	std::cout << "Any key to show the list" << std::endl;
	//getchar();
	
	for (const auto& application : applications) {
		//system("cls");
		//std::cout << "Any key to next, q to exit\n" << std::endl;

		std::cout << *application;
		//if (getchar() == 'y') break;
	}

	std::cout << "The end!" << std::endl;

	return 0;
}
