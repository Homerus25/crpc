#pragma once

#include <utility>
#include <future>
#include <iostream>

#include "deserialize_helper.h"
#include "rpc_fn.h"

#include "cista/serialization.h"

#include "message.h"
#include "ticket_store.h"
#include "receiver.h"

template<typename ReturnType>
class return_object {
public:
    explicit return_object(std::future<std::vector<unsigned char>>& fut)
        : future(std::move(fut))
    {}

    ReturnType operator()() {
        if(!future.valid()) {
          std::cout  << "future is invalid!" << std::endl;
        }
        auto res = future.get();
        if constexpr (!std::is_same_v<ReturnType, void>)
            return *cista::deserialize<ReturnType>(res);
    }

private:
    std::future<std::vector<unsigned char>> future;
};

template <typename Transport, typename Interface>
class rpc_async_client {
public:
    template <typename... Args>
    rpc_async_client(Args&&... args)  // NOLINT
    : transport{std::move(Receiver(ts_)), std::forward<Args>(args)...} {}


    template <typename ReturnType, typename... Args>
    return_object<ReturnType> call(fn<ReturnType, Args...> Interface::*const member_ptr,
                    Args&&... args) {
        std::future<std::vector<unsigned char>> response;
        response = sendMS(index_of_member(member_ptr), args...);
        return return_object<ReturnType>(response);
    }

    template < typename... Args>
    std::future<std::vector<unsigned char>> sendMS(unsigned fn_idx,
                                                 Args&&... args) {
        auto ticket_num = this->ts_.nextNumber();
        auto future = this->ts_.emplace(ticket_num);

        cista::byte_buf conTup = serializeArgs(std::forward<Args>(args)...);
        cista::offset::vector<unsigned char> conTup2 (conTup.begin(), conTup.end());

        message ms{
            ticket_num, fn_idx,
            conTup2
        };
        std::vector<uint8_t> ms_buf = serializeMessage(ms);
        message* tmp = cista::deserialize<message>(ms_buf);
        if(tmp->ticket_ != ticket_num || tmp->fn_idx != fn_idx) {
            std::cout << "cista failed at client send!!!" << std::endl;
        }

        transport.send(ms_buf);
        return future;
    }

    std::vector<uint8_t> serializeMessage(message& ms) const {
        try {
            return cista::serialize(ms);
        }catch (cista::cista_exception& ex) {
          std::cout << "send failed to serialize message: " << ex.what()
                    << std::endl;
        }
    }

    template < typename... Args>
    cista::byte_buf serializeArgs(Args&&... args) const {
        auto tup = std::make_tuple(std::forward<Args>(args)...);
        try {
          return cista::serialize(tup);
        }catch (cista::cista_exception& ex) {
            std::cout << "send failed to serialize arguments: " << ex.what() << std::endl;
        }
    }

    void stop() {
        transport.stop();
    }
  private:
    Transport transport;
    ticket_store ts_;
};
