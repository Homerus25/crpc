#include "Client.h"
#include "cista.h"

crpc::Client::Client(std::string name, unsigned int port)
    : endpoints(boost::asio::ip::address::from_string(name), port)
{
}

std::string crpc::Client::getDayTime()
{
    boost::asio::ip::tcp::socket socket(io);
    socket.connect(endpoints);

    for(;;) {
        std::vector<char> buffer(128);
        boost::system::error_code error;

        socket.write_some(boost::asio::buffer("getDayTime"), error);

        size_t len = socket.read_some(boost::asio::buffer(buffer), error);

        if (error == boost::asio::error::eof)
            break; // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error); // Some other error.

        auto* deserial = cista::offset::deserialize<cista::raw::string>(buffer);
        return std::string(*deserial);
    }

    return "";
}
