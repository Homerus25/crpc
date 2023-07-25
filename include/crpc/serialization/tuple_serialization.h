#pragma once

#include <memory>
#include "../message.h"
#include <variant>

struct TupleSerializer {
  typedef std::variant<int, double, std::string, std::vector<double>, std::vector<unsigned char>,
      std::tuple<int>, std::tuple<std::string>, std::tuple<std::vector<double>>, std::tuple<std::vector<unsigned char>>> SerializedServerContainer;
  typedef std::variant<message<SerializedServerContainer>> SerializedServerMessageContainer;

  typedef SerializedServerContainer SerializedClientContainer;
  typedef SerializedServerMessageContainer SerializedClientMessageContainer;

  template <class In>
  static auto serialize(In& in) {
    if constexpr (std::is_same_v<In, message<SerializedServerContainer>>) {
      return SerializedServerMessageContainer(std::move(in));
    }
    else {
      return SerializedServerContainer(std::move(in));
    }
  }

  template <class Out, class In>
  static auto deserialize(In& in) {
    return std::make_unique<Out>(std::move(std::get<Out>(in)));
  }
};