#pragma once

#include <vector>
#include <array>
#include <functional>

#include "cista/reflection/arity.h"
#include "cista/serialization.h"

#include "message.h"
#include "rpc_fn.h"

#include "log.h"

template<typename Interface>
class rpc_server {
public:
    std::vector<unsigned char> call(unsigned fn_idx,
                                    std::vector<unsigned char> const& params) {
        return this->fn_.at(fn_idx)(params);
    }

    template <typename BufferType>
    const std::optional<std::vector<u_int8_t>> process_message(BufferType buffer) {
      message* req = deserializeMessage(buffer);
      auto const func_num = req->fn_idx;
      if (func_num < this->fn_.size()) {
        auto const pload = this->call(
            func_num, std::vector<unsigned char>(req->payload_.begin(),
                                                 req->payload_.end()));
        message ms{
            req->ticket_, func_num,
            cista::offset::vector<unsigned char>(pload.begin(), pload.end())};

        return cista::serialize(ms);
      }

      return {};
    }

    template <typename BufferType>
    message* deserializeMessage(BufferType& buffer) const {
      try {
        return cista::deserialize<message>(buffer);
      }catch (cista::cista_exception& ex) {
        LogErr("error deserialze message on server: ", ex.what());
      }
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

