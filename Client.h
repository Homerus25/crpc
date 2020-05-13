#pragma once

#include <boost/asio.hpp>
#include "cista.h"

namespace crpc
{
    class Client {
    public:
        Client(const std::string& name, unsigned int port);

        template<int procName, typename resultType>
        resultType call();

        template<int procName, typename resultType, class ...ArgTypes>
        resultType call(ArgTypes...);

    private:
        boost::asio::io_context io;
        boost::asio::ip::tcp::endpoint endpoints;//(boost::asio::ip::address::from_string(host), 2000);

        std::vector<char> buffer;
        boost::system::error_code error;

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
            appendVector(buf, buf2);
            return buf;
        }
    };
}

template<int procName, typename resultType>
resultType crpc::Client::call()
{
    boost::asio::ip::tcp::socket socket(io);
    socket.connect(endpoints);

    std::vector<unsigned char> bu;
    cInt cint{procName};
    bu = cista::serialize(cint);

    for(;;) {
        socket.write_some(boost::asio::buffer(bu), error);

        size_t len = socket.read_some(boost::asio::buffer(buffer), error);

        if (error == boost::asio::error::eof)
            break; // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error); // Some other error.

        resultType* deserial = cista::offset::deserialize<resultType>(buffer);
        return *deserial;
    }

    throw std::exception();
}


template<int procName, typename resultType, class... ArgTypes>
resultType crpc::Client::call(ArgTypes... args)
{
    boost::asio::ip::tcp::socket socket(io);
    socket.connect(endpoints);

    std::vector<unsigned char> bu;
    bu = serialize(procName, args...);

    for(;;) {
        socket.write_some(boost::asio::buffer(bu), error);

        size_t len = socket.read_some(boost::asio::buffer(buffer), error);

        if (error == boost::asio::error::eof)
            break; // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error); // Some other error.

        resultType* deserial = cista::offset::deserialize<resultType>(buffer);
        return *deserial;
    }

    throw std::exception();
}
