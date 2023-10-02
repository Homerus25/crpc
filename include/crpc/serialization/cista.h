#pragma once

#include "../deserialize_helper.h"
#include "cista/serialization.h"

#include "../log.h"

struct CistaSerialzer {
  typedef cista::offset::vector<unsigned char>  SerializedServerContainer;
  typedef SerializedServerContainer SerializedServerMessageContainer;

  typedef SerializedServerContainer SerializedClientContainer;
  typedef SerializedServerMessageContainer SerializedClientMessageContainer;

  template <class In>
  static SerializedServerContainer serialize(In& in) {
    try {
      cista::buf<SerializedServerContainer> buf;
      cista::serialize<cista::mode::NONE, cista::buf<SerializedServerContainer>>(buf, in);
      return buf.buf_;
    }catch (cista::cista_exception& ex) {
      LogErr("send failed to serialize message: ", ex.what());
    }
  }

  template <class Out, class In>
  static auto deserialize(In& in) {
    try {
      return cista::deserialize<Out, cista::mode::CAST>(in);
    }catch (cista::cista_exception& ex) {
      LogErr("error deserialze message on server: ", ex.what());
    }
  }
};