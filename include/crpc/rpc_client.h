#pragma once

#include <utility>
#include <vector>

#include "cista/reflection/arity.h"
#include "cista/serialization.h"

#include "rpc_fn.h"

#include "tuple_deserialize.h"


template <typename Transport, typename Interface>
class rpc_client : public Transport {
public:
    template <typename... Args>
    rpc_client(Args&&... args)  // NOLINT
    : Transport{std::forward<Args>(args)...} {}

    template <typename ReturnType, typename... Args>
    ReturnType call(fn<ReturnType, Args...> Interface::*const member_ptr,
                    Args&&... args) {
        std::vector<unsigned char> response;
        if constexpr (sizeof...(Args) == 0U) {
            response = Transport::send(index_of_member(member_ptr), {});
        } else {
            auto const params = std::make_tuple(std::forward<Args>(args)...);
            response = Transport::send(index_of_member(member_ptr),
                                       cista::serialize(params));
        }
        if constexpr (!std::is_same_v<
                      ReturnType,
                void>) {  // NOLINT(bugprone-suspicious-semicolon)
            return *cista::deserialize<ReturnType>(response);
        }
    }
};
