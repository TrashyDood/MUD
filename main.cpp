#include "include/Server.h"

int main()
{
    Server server;
    server.Run();

    server.RunningTask().wait();
}