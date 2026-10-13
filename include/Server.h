# pragma once

#include <iostream>
#include <string>
#include <chrono> 
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

class Server
{
public:
    static constexpr unsigned short DefaultPort = 8080;

    Server();

    void Run();
};