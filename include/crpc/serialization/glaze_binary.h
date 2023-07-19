#pragma once

#include <glaze/json.hpp>
#include <string>
#include <memory>

struct GlazeBinarySerializer {
  typedef std::string SerializedContainer;

  template <class In>
  static SerializedContainer serialize(In& in) {
    return glz::write_binary(in);
  }

  template <class Out, class In>
  static auto deserialize(In& in) {
    return std::make_shared<Out>(glz::read_binary<Out>(in).value());
  }
};