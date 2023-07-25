#pragma once

#include <array>
#include <functional>
#include <optional>
#include <vector>

#include "message.h"
#include "rpc_fn.h"

#include "log.h"

template<typename Interface, typename Serializer>
class rpc_server {
public:
    Serializer::SerializedServerContainer call(unsigned fn_idx,
                                       Serializer::SerializedClientContainer const& params) {
        return this->fn_.at(fn_idx)(params);
    }

    template <typename BufferType>
    const std::optional<typename Serializer::SerializedServerMessageContainer> process_message(BufferType buffer) {
      auto req = Serializer::template deserialize<message<typename Serializer::SerializedClientContainer>>(buffer);
      auto const func_num = req->fn_idx;
      if (func_num < this->fn_.size()) {
        auto pload = this->call(func_num, req->payload_);
        message<typename Serializer::SerializedServerContainer> ms{
            req->ticket_, func_num,
            pload
        };

        return Serializer::serialize(ms);
      }

      return {};
    }

    template <typename Fn, typename ReturnType, typename... Args>
      void reg(fn<ReturnType, Args...> Interface::*const member_ptr, Fn&& f) {
        fn_[index_of_member(member_ptr)] =
                [mf = std::forward<Fn>(f)](Serializer::SerializedClientContainer const& in)
                  -> Serializer::SerializedServerContainer {
                    if constexpr (std::is_same_v<ReturnType, void>) {
                        if constexpr (sizeof...(Args) == 0) {
                            mf();
                        } else {
                            std::apply(mf, *Serializer::template deserialize<std::tuple<Args...>>(in));
                        }
                        return {};
                    } else {
                        if constexpr (sizeof...(Args) == 0) {
                            auto const return_value = mf();
                            return Serializer::serialize(return_value);
                        } else {
                            auto const return_value =
                            std::apply(mf, *Serializer::template deserialize<std::tuple<Args...>>(in));

                            return Serializer::serialize(return_value);
                        }
                    }
                };
    }

protected:
  std::array<
      std::function<typename Serializer::SerializedServerContainer(typename Serializer::SerializedClientContainer const&)>,
      cista::arity<Interface>()> fn_;
};

