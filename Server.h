#pragma once

#include <boost/asio.hpp>
#include "cista.h"


template<typename RetType>
std::vector<unsigned char> stub(std::function<RetType(void)> funcPtr)
{
    auto ret = funcPtr();
    return cista::serialize(ret);
}

/*
template<typename RetType, class ...ArgumentTypes>
std::vector<unsigned char> stub(std::function<RetType(ArgumentTypes...)> funcPtr, std::vector<char> argsBuf)
{
    std::tuple<ArgumentTypes...> params = *(cista::deserialize<std::tuple<ArgumentTypes...>>(argsBuf));
    RetType ret = std::apply(funcPtr, params);
    return cista::serialize(ret);
}
*/


/*
template<typename retType, class ...ArgumentTypes>
cista::raw::string stub(void* funcPtr, std::vector<char> argsBuf)
{
    auto params = cista::deserialize<std::tuple<ArgumentTypes...>>(argsBuf);
    auto ret = std::apply(static_cast<std::function<retType(ArgumentTypes...)>>(funcPtr), params);
    return cista::serialize(ret);
}
 */

namespace crpc
{
    class Server {
    public:
        explicit Server(unsigned int port);

        void run();

    private:
        boost::asio::io_context io;
        boost::asio::ip::tcp::acceptor acceptor;

        std::vector<std::function<std::vector<unsigned char>(std::vector<char>)>> funcs;

    public:
        template<typename retType>
        void registerFunction(std::function<retType(void)> funcP)
        {
            //std::function<std::vector<unsigned char>(std::function<retType(void)>)> stubFunc = stub<retType>;
            auto stubFunc = stub<retType>;

            std::function<std::vector<unsigned char>(std::vector<char>)> f = [=] (std::vector<char> v)
            {
                return stubFunc(funcP);
            };

            funcs.emplace_back(f);

        }

        /*
        template<typename retType, class ...ArgTypes>
        void registerFunction(std::function<retType(ArgTypes...)> funcP)
        {
            auto stubFunc = stub<retType, ArgTypes...>;
            std::function<std::vector<unsigned char>(std::vector<char>)> f = [=] (std::vector<char> v)
            {
                return stubFunc(funcP, v);
            };
            funcs.emplace_back(std::bind_front(stubFunc, funcP));
        }
        */
    };

}
