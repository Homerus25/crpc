#include "Server.h"

#include <functional>
#include <iostream>

cista::offset::string make_daytime_string()
{
    std::time_t now = time(0);
    return std::ctime(&now);
}

int addTwoNums(int a, int b)
{
    std::cout << "add two nums: " << a << " " << b << "\n";
    return a + b;
}

int main(int argc, char* argv[])
{
    crpc::Server server(2000);

    //std::function<cista::offset::string(void)> func = make_daytime_string;
    //server.registerFunction<cista::offset::string>(func);
    server.registerFunction<cista::offset::string>(make_daytime_string);

    std::function<int(int, int)> func2 = addTwoNums;
    server.registerFunctionArgs<int, int, int>(func2);
    //server.registerFunctionArgs<int, int, int>(addTwoNums);


    server.run();
    //server.test();

    return 0;
}
