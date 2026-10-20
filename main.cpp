#include "include/Client.h"
#include "include/Server.h"

#include <charconv>
#include <cstring>
#include <iostream>

int main()
{
    auto server = Server::CreateServer();

    server->Run();

    return 0;
}

// int main(int argc, char* argv[])
// {
//     return 0;
// }