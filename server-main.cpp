#include "Server.h"

#include <string>
#include <vector>
#include <functional>
#include <iostream>

cista::offset::string make_daytime_string()
{
    std::time_t now = time(0);
    return std::ctime(&now);
}

/*
int addTwoNums(int a, int b)
{
    return a + b;
}
*/

int main(int argc, char* argv[])
{
    crpc::Server server(2000);

    //std::function<cista::offset::string(void)> func = make_daytime_string;
    //server.registerFunction<cista::offset::string>(func);
    server.registerFunction<cista::offset::string>(make_daytime_string);

    server.run();

    return 0;
}
