#pragma once

#include <any>

struct NoSerializer {
  typedef std::any SerializedContainer;

  template <class In>
  static SerializedContainer serialize(In& in) {
    return std::any(in);
  }

  template <class Out, class In>
  static auto deserialize(In& in) {
    return std::make_shared<Out>(std::any_cast<Out>(in));
  }
};