#pragma once

#include "rpc_async_client.h"
#include "boost/asio.hpp"
#include "message.h"

class async_asio_transport {
public:
    explicit async_asio_transport(std::string const& name, unsigned int const port)
            : endpoints_(boost::asio::ip::address::from_string(name), port)
            , ticket_num_(0)
    {
        socket_.connect(endpoints_);
        //std::async(std::launch::async, &async_asio_transport::receive, this);
        std::thread t(&async_asio_transport::receive, this);
        t.detach();
    }

    std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                    std::vector<unsigned char> const& params) {

        message ms{ticket_num_++, fn_idx, cista::offset::vector<unsigned char>(params.begin(), params.end())};

        std::promise<std::vector<unsigned char>> promise;
        auto future = promise.get_future();
        tickets_.emplace(ms.ticket_, std::move(promise));
        //return promise.get_future();

        write(socket_, boost::asio::buffer(cista::serialize(ms)));

        return future;

        /*
        std::vector<unsigned char> buffer(1024);
        boost::system::error_code error;
        socket_.read_some(boost::asio::buffer(buffer), error);

        if(error) {
            std::cerr << "error while reading\n";
            throw boost::system::system_error(error);
        }
        return buffer;
         */

    }

    void receive() {
        while(1) {
            std::vector<unsigned char> buffer(1024);
            boost::system::error_code error;
            auto ms_size = socket_.read_some(boost::asio::buffer(buffer), error);

            if(error) {
                std::cerr << "error while reading\n";
                throw boost::system::system_error(error);
            }

            auto ms = cista::deserialize<message>(buffer);
            /*
            std::cout << "ms-size: " << ms_size << " ticket: " << ms->ticket_ << " tickets_.size:" << tickets_.size() <<  std::endl;// << "\n";
            for(auto beg = tickets_.begin(); beg != tickets_.end(); ++beg)
                std::cout << beg->first << "\n";
                */
            tickets_.at(ms->ticket_).set_value(std::vector<unsigned char>(ms->payload_.begin(), ms->payload_.end()));
        }
    }

    ~async_asio_transport()
    {
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        socket_.close();
    }

private:
    boost::asio::io_context io_;
    boost::asio::ip::tcp::endpoint endpoints_;

    boost::asio::ip::tcp::socket socket_{io_};

    std::atomic<uint64_t> ticket_num_;
    cista::raw::hash_map<uint64_t, std::promise<std::vector<unsigned char>>> tickets_;
};

template<typename Interface>
using rpc_async_asio_client = rpc_async_client<async_asio_transport, Interface>;
