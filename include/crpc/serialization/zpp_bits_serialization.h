#pragma once

#include <memory>
#include "zpp_bits/zpp_bits.h"

struct ZppBitsSerializer {
  typedef std::vector<std::byte>  SerializedContainer;

  template <class In>
  static SerializedContainer serialize(In& in) {
    SerializedContainer res;
    zpp::bits::out gout(res);
    gout(in);
    return res;
  }

  template <class Out, class In>
  static auto deserialize(In& in) {
    zpp::bits::in gin(in);
    Out res;
    gin(res);
    return std::make_shared<Out>(res);
  }
};