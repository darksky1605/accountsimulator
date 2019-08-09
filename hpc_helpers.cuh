#ifndef HPC_HELPERS_CUH
#define HPC_HELPERS_CUH

#include <iostream>
#include <cstdint>
#include <chrono>

#define TIMERSTARTCPU(label)                                                  \
	std::chrono::time_point<std::chrono::system_clock> a##label, b##label; \
	a##label = std::chrono::system_clock::now();

#define TIMERSTOPCPU(label)                                                   \
	b##label = std::chrono::system_clock::now();                           \
	std::chrono::duration<double> delta##label = b##label-a##label;        \
	std::cout << "# elapsed time ("<< #label <<"): "                       \
				<< delta##label.count()  << " s" << std::endl;

// safe division
#define SDIV(x,y)(((x)+(y)-1)/(y))

#endif
