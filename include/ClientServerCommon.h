#pragma once

#include <iostream>
#include <cstdint>

#include "PacketsIO.h"
#include "ThreadPool.h"

#define WRITE(x) std::cout << x
#define WRITE_LINE(x) WRITE(x) << "\n"
#define WRITE_ERROR(x) std::cerr << x << "\n"

using Port = uint16_t;

//inline ThreadPool<64>& StaticThreadPool() { static ThreadPool<64> threadPoolInstance; return threadPoolInstance; }