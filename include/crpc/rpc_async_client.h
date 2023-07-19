#pragma once

#include <utility>
#include <future>
#include <iostream>

#include "rpc_fn.h"

#include "message.h"
#include "ticket_store.h"
#include "receiver.h"

template<typename ReturnType, typename Serializer>
class return_object {
public:
    explicit return_object(std::future<typename Serializer::SerializedContainer>& fut)
        : future(std::move(fut))
    {}

    ReturnType operator()() {
        if(!future.valid()) {
          std::cout  << "future is invalid!" << std::endl;
        }
        auto res = future.get();
        if constexpr (!std::is_same_v<ReturnType, void>)
          return *Serializer::template deserialize<ReturnType>(res);
    }

private:
    std::future<typename Serializer::SerializedContainer> future;
};

template <typename Transport, typename Interface, typename Serializer>
class rpc_async_client {
public:
    template <typename... Args>
    rpc_async_client(Args&&... args)  // NOLINT
    : transport{std::move(Receiver<Serializer>(ts_)), std::forward<Args>(args)...} {}


    template <typename ReturnType, typename... Args>
    return_object<ReturnType, Serializer> call(fn<ReturnType, Args...> Interface::*const member_ptr,
                    Args&&... args) {
        std::future<typename Serializer::SerializedContainer> response;
        response = sendMS(index_of_member(member_ptr), args...);
        return return_object<ReturnType, Serializer>(response);
    }

    template < typename... Args>
    std::future<typename Serializer::SerializedContainer> sendMS(unsigned fn_idx,
                                                 Args&&... args) {
        auto ticket_num = this->ts_.nextNumber();
        auto future = this->ts_.emplace(ticket_num);

        auto conTup = serializeArgs(std::forward<Args>(args)...);

        message ms{
            ticket_num, fn_idx,
            conTup
        };
        auto ms_buf = serializeMessage(ms);

        transport.send(ms_buf);
        return future;
    }

    auto serializeMessage(message<typename Serializer::SerializedContainer>& ms) const {
        return Serializer::serialize(ms);
    }

    template < typename... Args>
    auto serializeArgs(Args&&... args) const {
        auto tup = std::make_tuple(std::forward<Args>(args)...);
        return Serializer::serialize(tup);
    }

    void stop() {
        transport.stop();
    }
  private:
    Transport transport;
    ticket_store<typename Serializer::SerializedContainer> ts_;
};
