#include "include/Client.h"
#include "include/Server.h"

#include <charconv>
#include <cstring>
#include <iostream>

int main()
{
    Server server;

    server.Run();

    return 0;
}

// int main(int argc, char* argv[])
// {
//     return 0;
// }