#include "include/Server.h"
#include "include/Utils.h"

int main()
{
    FixedBitBuffer buffer;
    uint8_t value1 = 30, value2 = 255;

    buffer.WriteNext(value1);
    buffer.WriteNext(value2);

    value1 = 0;
    value2 = 0;

    buffer.ReadNext(value1);
    buffer.ReadNext(value2);

    WRITE_LINE((int)(value1));
    WRITE_LINE((int)(value2));

    Server server;
    server.Run();
}