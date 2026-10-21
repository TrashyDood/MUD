#include "include/Client.h"
#include "include/Server.h"

#include <charconv>
#include <cstring>
#include <iostream>

int main()
{
    Server server;

    int value1 = 50;
    int value2 = 45;
    size_t offset = 0;

    ByteBufferIO::ByteBuffer24 buffer;
    buffer.Write(value1, offset);
    buffer.Write(value2, offset, offset);


    server.Run();

    return 0;
}

// int main(int argc, char* argv[])
// {
//     return 0;
// }