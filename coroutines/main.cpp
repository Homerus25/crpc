#include "config.hpp"
#include "app.hpp"

#include <iostream>

int main()
{
    using namespace project;

    net::io_context ioc;

    //auto the_app = app(ioc.get_executor());
    //auto the_app = app(ioc);
    app the_app(ioc);
    the_app.run();   // initiate async ops

    try
    {
        ioc.run();
    }
    catch(std::exception& e)
    {
        std::cout << "program bombed: " << e.what();
    }
}