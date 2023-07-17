#pragma once

#include <vector>
#include <array>
#include <functional>

#include "serialization/cista.h"

#include "message.h"
#include "rpc_fn.h"

#include "log.h"

template<typename Interface, typename Serializer>
class rpc_server {
private:
  typedef Serializer::SerializedContainer SerializedContainer;

public:
    Serializer::SerializedContainer call(unsigned fn_idx,
                                       Serializer::SerializedContainer const& params) {
        return this->fn_.at(fn_idx)(params);
    }

    template <typename BufferType>
    const std::optional<SerializedContainer> process_message(BufferType buffer) {
      message<SerializedContainer>* req = deserializeMessage(buffer);
      auto const func_num = req->fn_idx;
      if (func_num < this->fn_.size()) {
        auto pload = this->call(func_num, req->payload_);
        message ms{
            req->ticket_, func_num,
            pload
        };

        return Serializer::serialize(ms);
      }

      return {};
    }

    template <typename BufferType>
    message<SerializedContainer>* deserializeMessage(BufferType& buffer) const {
      return Serializer::template deserialize<message<SerializedContainer>>(buffer);
    }

    template <typename Fn, typename ReturnType, typename... Args>
      void reg(fn<ReturnType, Args...> Interface::*const member_ptr, Fn&& f) {
        fn_[index_of_member(member_ptr)] =
                [mf = std::forward<Fn>(f)](Serializer::SerializedContainer const& in)
                  -> Serializer::SerializedContainer {
                    if constexpr (std::is_same_v<ReturnType, void>) {
                        if constexpr (sizeof...(Args) == 0) {
                            mf();
                        } else {
                            std::apply(mf, *CistaSerialzer::deserialize<std::tuple<Args...>>(in));
                        }
                        return {};
                    } else {
                        if constexpr (sizeof...(Args) == 0) {
                            auto const return_value = mf();
                            return CistaSerialzer::serialize(return_value);
                        } else {
                            auto const return_value =
                            std::apply(mf, *CistaSerialzer::deserialize<std::tuple<Args...>>(in));

                            return CistaSerialzer::serialize(return_value);
                        }
                    }
                };
    }

protected:
  std::array<
      std::function<SerializedContainer(SerializedContainer const&)>,
      cista::arity<Interface>()> fn_;
};

