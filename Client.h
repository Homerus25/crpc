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
        boost::asio::ip::tcp::endpoint endpoints;//(boost::asio::ip::address::from_string(host), 2000);

        std::vector<char> buffer;
        boost::system::error_code error;

        boost::asio::ip::tcp::socket socket;//(io);
        //socket.connect(endpoints);

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
resultType crpc::Client::callNoParameter()
{
    std::vector<unsigned char> bu;
    cInt cint{procName};
    bu = cista::serialize(cint);

    socket.write_some(boost::asio::buffer(bu), error); write(socket, boost::asio::buffer(bu), error);
    size_t len = socket.read_some(boost::asio::buffer(buffer), error);

    if (error)
        throw boost::system::system_error(error); // Some other error.


    resultType *deserial = cista::offset::deserialize<resultType>(buffer);
    return *deserial;
}

template<int procName, typename resultType, class ...ArgTypes>
resultType crpc::Client::call(ArgTypes... args)
{
    std::vector<unsigned char> bu;
    bu = serialize(procName, args...);

    //socket.write_some(boost::asio::buffer(bu), error);
    write(socket, boost::asio::buffer(bu), error);

    size_t len = socket.read_some(boost::asio::buffer(buffer), error);

    if (error)
        throw boost::system::system_error(error); // Some other error.

    resultType *deserial = cista::offset::deserialize<resultType>(buffer);
    return *deserial;
}


template<int procName, class... ArgTypes>
void crpc::Client::callNoReturn(ArgTypes... args)
{
    std::vector<unsigned char> bu;
    bu = serialize(procName, args...);

    //socket.write_some(boost::asio::buffer(bu), error);
    write(socket, boost::asio::buffer(bu), error);

    if (error == boost::asio::error::eof)
        return;
    else if (error)
        throw boost::system::system_error(error); // Some other error.
}


template<int procName>
void crpc::Client::callNoReturnNoArgs()
{
    std::vector<unsigned char> bu;
    bu = serialize(procName);

    //socket.write_some(boost::asio::buffer(bu), error);
    write(socket, boost::asio::buffer(bu), error);

    if (error == boost::asio::error::eof)
        return;
    else if (error)
        throw boost::system::system_error(error); // Some other error.
}

