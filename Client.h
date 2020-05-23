#pragma once

#include <boost/asio.hpp>
#include "cista.h"

namespace crpc
{
    class Client {
    public:
        Client(const std::string& name, unsigned int port);

        template<int procName, typename resultType>
        resultType callNoParameter();

        template<int procName, typename resultType, class ...ArgTypes>
        resultType call(ArgTypes...);

        template<int procName, class ...ArgTypes>
        void callNoReturn(ArgTypes...);

        template<int procName>
        void callNoReturnNoArgs();

    private:
        boost::asio::io_context io;
        boost::asio::ip::tcp::endpoint endpoints;

        std::vector<char> buffer;

        boost::asio::ip::tcp::socket socket;//(io);

        struct cInt { int a; };

        template<typename Type>
        void appendVector(std::vector<Type>& a, std::vector<Type>& b)
        {
            auto offset = a.size();
            a.resize(a.size() + b.size());
            std::copy(b.begin(), b.end(), a.begin() + offset);
        }

        template<typename ArgType>
        std::vector<unsigned char> serialize(ArgType arg)
        {
            std::vector<unsigned char> buf = cista::serialize(arg);
            return buf;
        }

        template<typename Ag1, typename ...Args>
        std::vector<unsigned char> serialize(Ag1 arg1, Args...args)
        {
            std::vector<unsigned char> buf = serialize(arg1);
            auto buf2 = serialize(args...);
            appendVector(buf, buf2); // maybe there is a more performant way!?
            return buf;
        }
    };
}

template<int procName, typename resultType>
resultType crpc::Client::callNoParameter()
{
    std::vector<unsigned char> bu = serialize(procName);
    write(socket, boost::asio::buffer(bu));

    socket.read_some(boost::asio::buffer(buffer));
    return *cista::offset::deserialize<resultType>(buffer);
}

template<int procName, typename resultType, class ...ArgTypes>
resultType crpc::Client::call(ArgTypes... args)
{
    std::vector<unsigned char> bu = serialize(procName, args...);
    write(socket, boost::asio::buffer(bu));

    socket.read_some(boost::asio::buffer(buffer));
    return *cista::offset::deserialize<resultType>(buffer);
}

template<int procName, class... ArgTypes>
void crpc::Client::callNoReturn(ArgTypes... args)
{
    std::vector<unsigned char> bu = serialize(procName, args...);
    write(socket, boost::asio::buffer(bu));
}

template<int procName>
void crpc::Client::callNoReturnNoArgs()
{
    std::vector<unsigned char> bu = serialize(procName);
    write(socket, boost::asio::buffer(bu));
}

