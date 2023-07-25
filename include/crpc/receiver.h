#pragma once

#include "ticket_store.h"
#include "message.h"

template<class Serializer>
class Receiver {
private:
  typedef message<typename Serializer::SerializedServerContainer> message_t;

public:
  Receiver(ticket_store<typename Serializer::SerializedServerContainer>& ts) : ts_(ts)
  {}

  template <class Container>
  void processAnswer(Container answer) {
    auto ms = Serializer::template deserialize<message_t>(answer);
    this->ts_.setValue(ms->ticket_, ms->payload_);
  }

private:
  ticket_store<typename Serializer::SerializedServerContainer>& ts_;
};