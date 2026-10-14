#pragma once

#include <iostream>

#include "../include/CommandsIO.h"
#include "../include/ThreadPool.h"

#define WRITE(x) std::cout << x
#define WRITE_LINE(x) WRITE(x) << "\n"
#define WRITE_ERROR(x) std::cerr << x << "\n"

using Port = unsigned int;
using CommandPacket = unsigned long long;

enum CommandType : uint8_t
{
    Message = 0,
    Attack = 1,
    Quit = 2,
    Count = 3
};

//inline ThreadPool<64>& StaticThreadPool() { static ThreadPool<64> threadPoolInstance; return threadPoolInstance; }