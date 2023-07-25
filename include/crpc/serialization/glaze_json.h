#pragma once

#include <glaze/json.hpp>
#include <string>
#include <memory>
#include "../message.h"

template <>
struct glz::meta<message<std::string>> {
  using T = message<std::string>;
  static constexpr auto value = object(
      "ticket", &T::ticket_,
      "function", &T::fn_idx,
      "payload", &T::payload_
  );
};

struct GlazeJSONSerializer {
  typedef std::string SerializedServerContainer;
  typedef SerializedServerContainer SerializedServerMessageContainer;

  typedef SerializedServerContainer SerializedClientContainer;
  typedef SerializedServerMessageContainer SerializedClientMessageContainer;

  template <class In>
  static SerializedServerContainer serialize(In& in) {
    return glz::write_json(in);
  }

  template <class Out, class In>
  static auto deserialize(In& in) {
    return std::make_shared<Out>(glz::read_json<Out>(in).value());
  }
};