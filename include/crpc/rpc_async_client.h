#pragma once

#include <utility>
#include <future>
#include <iostream>

#include "deserialize_helper.h"
#include "rpc_fn.h"

#include "cista/serialization.h"

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
class rpc_async_client : public Transport {
public:
    template <typename... Args>
    rpc_async_client(Args&&... args)  // NOLINT
    : Transport{std::forward<Args>(args)...} {}


    template <typename ReturnType, typename... Args>
    return_object<ReturnType> call(fn<ReturnType, Args...> Interface::*const member_ptr,
                    Args&&... args) {
        std::future<std::vector<unsigned char>> response;
        if constexpr (sizeof...(Args) == 0U) {
            response = Transport::send(index_of_member(member_ptr), {});
        } else {
            auto const params = std::make_tuple(std::forward<Args>(args)...);
            response = Transport::send(index_of_member(member_ptr),
                                       cista::serialize(params));
        }
        return return_object<ReturnType>(response);
    }
};
