#pragma once

#include <boost/asio.hpp>
#include "cista.h"

namespace crpc
{

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

    private:
        void listenToClient(boost::asio::ip::tcp::socket&);

    private:
        template<typename RetType>
        static std::vector<unsigned char> stubNoArguments(std::function<RetType(void)> funcPtr)
        {
            auto ret = funcPtr();
            return cista::serialize(ret);
        }

        template<typename RetType, class ...ArgumentTypes>
        static std::vector<unsigned char> stub(std::function<RetType(ArgumentTypes...)> funcPtr, std::vector<unsigned char> argsBuf)
        {
            std::tuple<ArgumentTypes...> params = *(cista::deserialize<std::tuple<ArgumentTypes...>>(argsBuf));
            RetType ret = std::apply(funcPtr, params);
            return cista::serialize(ret);
        }

        template<class ...ArgumentTypes>
        static void stubNoReturn(std::function<void(ArgumentTypes...)> funcPtr, std::vector<unsigned char> argsBuf)
        {
            std::tuple<ArgumentTypes...> params = *(cista::deserialize<std::tuple<ArgumentTypes...>>(argsBuf));
            std::apply(funcPtr, params);
        }

        /*
        static void stubNoReturnNoParamter(std::function<void(void)> funcPtr)
        {
            std::tuple<ArgumentTypes...> params = *(cista::deserialize<std::tuple<ArgumentTypes...>>(argsBuf));
            std::apply(funcPtr, params);
        }
         */

    public:
        template<typename returnType>
        void registerFunction(std::function<returnType(void)> funcP)
        {
            funcs.emplace_back([=] () { return stubNoArguments<returnType>(funcP); });
        }

        template<typename returnType, class ...ArgTypes>
        void registerFunctionArgs(std::function<returnType(ArgTypes...)>& funcP)
        {
            funcs.emplace_back([&, funcP] () { return stub<returnType, ArgTypes...>(funcP, params); });
        }

        template<class ...ArgTypes>
        void registerFunctionNoReturn(std::function<void(ArgTypes...)>& funcP)
        {
            funcs.emplace_back([&, funcP] () { stubNoReturn(funcP, params); return std::vector<unsigned char>(); });
        }

        void registerFunctionNoReturnNoParameter(std::function<void(void)>& funcP)
        {
            funcs.emplace_back([&, funcP] () { funcP(); return std::vector<unsigned char>(); });
        }

    };
}
