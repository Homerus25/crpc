#include "server.h"

int main(int argc, char* argv[])
{
    crpc::server server(2000);
    server.run();

    return 0;
}
