#pragma once

#include <vector>
#include <array>
#include <functional>

#include "cista/reflection/arity.h"
#include "cista/serialization.h"

#include "rpc_fn.h"


template<typename Interface>
class rpc_server {
public:
    std::vector<unsigned char> call(unsigned fn_idx,
                                    std::vector<unsigned char> const& params) {
        return fn_.at(fn_idx)(params);
    }

    template <typename Fn, typename ReturnType, typename... Args>
    void reg(fn<ReturnType, Args...> Interface::*const member_ptr, Fn&& f) {
        fn_[index_of_member(member_ptr)] =
                [mf = std::forward<Fn>(f)](std::vector<unsigned char> const& in)
                        -> std::vector<unsigned char> {
                    if constexpr (std::is_same_v<ReturnType, void>) {
                        if constexpr (sizeof...(Args) == 0) {
                            mf();
                        } else {
                            std::apply(mf, *cista::deserialize<std::tuple<Args...>>(in));
                        }
                        return {};
                    } else {
                        if constexpr (sizeof...(Args) == 0) {
                            auto const return_value = mf();
                            return cista::serialize(return_value);
                        } else {
                            auto const return_value =
                            std::apply(mf, *cista::deserialize<std::tuple<Args...>>(in));
                            return cista::serialize(return_value);
                        }
                    }
                };
    }

protected:
    std::array<std::function<std::vector<unsigned char>(
            std::vector<unsigned char> const&)>,
    cista::arity<Interface>()>
    fn_;
};

