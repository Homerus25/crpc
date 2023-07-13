#pragma once

#include "ticket_store.h"
#include "message.h"

class Receiver {
public:
  Receiver(ticket_store& ts) : ts_(ts)
  {}

  void processAnswer(std::vector<u_int8_t > answer) {
    const message* ms = deserialzeMessage(answer);
    this->ts_.setValue(ms->ticket_, ms->payload_);
  }

private:
  const message* const deserialzeMessage(const std::vector<u_int8_t>& answer) const {
    try {
      return cista::deserialize<message>(answer);
    }catch (cista::cista_exception& ex) {
      std::cout << "error deserializing message in receiver: " << ex.what() << std::endl;
    }
  }

  ticket_store& ts_;
};