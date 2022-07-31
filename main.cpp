#include <enet/enet.h>

#include "Networking/ENet_Common.h"
#include "Server/ENetServer.h"

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

int main(int argc, char** argv)
{
    DbConnector* dbConnector = new DbConnector();
    dbConnector->Connect();

    ENetServer server(8000, dbConnector);

    if(!server.Start())
        return EXIT_FAILURE;

    while(1)
    {
        server.Update();
    }

    server.Stop();
    return EXIT_SUCCESS;
}