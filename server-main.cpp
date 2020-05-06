#include "Server.h"

int main(int argc, char* argv[])
{
    crpc::Server server(2000);
    server.run();

    return 0;
}
