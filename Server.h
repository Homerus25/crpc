#pragma once

#include <boost/asio.hpp>
#include "cista.h"

namespace crpc
{
    template<typename RetType>
    std::vector<unsigned char> stub(std::function<RetType(void)> funcPtr)
    {
        auto ret = funcPtr();
        return cista::serialize(ret);
    }

    template<typename RetType, class ...ArgumentTypes>
    std::vector<unsigned char> stubArgs(std::function<RetType(ArgumentTypes...)> funcPtr, std::vector<unsigned char> argsBuf)
    {
        std::tuple<ArgumentTypes...> params = *(cista::deserialize<std::tuple<ArgumentTypes...>>(argsBuf));
        RetType ret = std::apply(funcPtr, params);
        return cista::serialize(ret);
    }

    class Server {
    public:
        explicit Server(unsigned int port);

        void run();
        void test();

    private:
        boost::asio::io_context io;
        boost::asio::ip::tcp::acceptor acceptor;

        std::vector<std::function<std::vector<unsigned char>(void)>> funcs;

        std::vector<char> buffer;
        std::vector<unsigned char> params;

    public:
        template<typename retType>
        void registerFunction(std::function<retType(void)> funcP)
        {
            auto stubFunc = stub<retType>;

            std::function<std::vector<unsigned char>(void)> f = [=] ()
            {
                return stubFunc(funcP);
            };

            funcs.emplace_back(f);
        }

        template<typename retType, class ...ArgTypes>
        void registerFunctionArgs(std::function<retType(ArgTypes...)>& funcP)
        {
            funcs.push_back([&, funcP] () { return stubArgs<retType, ArgTypes...>(funcP, params); });
        }
    };
}
