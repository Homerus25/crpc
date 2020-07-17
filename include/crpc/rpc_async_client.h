#pragma once

#include <utility>
#include <future>

//#include "rpc_client.h"

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
        //std::vector<unsigned char> response;
        std::promise<return_object<ReturnType>> promise;
        std::future<std::vector<unsigned char>> response;
        if constexpr (sizeof...(Args) == 0U) {
            response = Transport::send(index_of_member(member_ptr), {});
        } else {
            auto const params = std::make_tuple(std::forward<Args>(args)...);
            response = Transport::send(index_of_member(member_ptr),
                                       cista::serialize(params));
        }
        return return_object<ReturnType>(response);
        //return [std::future<std::vector<unsigned char>> res = std::move(response)]() { return cista::deserialize<ReturnType>(res.get()); };
        //return [res = std::move(response)]() { return cista::deserialize<ReturnType>(res.get()); };
        /*
        if constexpr (!std::is_same_v<
                ReturnType,
                void>) {  // NOLINT(bugprone-suspicious-semicolon)
            return *cista::deserialize<ReturnType>(response);
        }
         */
    }
};
