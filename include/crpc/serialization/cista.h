#pragma once

#include "../deserialize_helper.h"
#include "cista/serialization.h"

#include "../log.h"

struct CistaSerialzer {
  typedef cista::offset::vector<unsigned char>  SerializedContainer;

  template <class In>
  static SerializedContainer serialize(In& in) {
    try {
      auto con = cista::serialize(in);
      return SerializedContainer(con.begin(), con.end());
    }catch (cista::cista_exception& ex) {
      LogErr("send failed to serialize message: ", ex.what());
    }
  }

  template <class Out, class In>
  static auto deserialize(In& in) {
    try {
      return cista::deserialize<Out>(in);
    }catch (cista::cista_exception& ex) {
      LogErr("error deserialze message on server: ", ex.what());
    }
  }
};