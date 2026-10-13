#pragma once
#include <iostream>

#define WRITE(x) std::cout << x
#define WRITELINE(x) WRITE(x) << "\n"
#define WRITE_ERROR(x) std::cerr << x << "\n"

using Port = unsigned int;