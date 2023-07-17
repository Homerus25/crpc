#pragma once

#include "ticket_store.h"
#include "message.h"

template<class Serializer>
class Receiver {
private:
  typedef message<typename Serializer::SerializedContainer> message_t;

public:
  Receiver(ticket_store<typename Serializer::SerializedContainer>& ts) : ts_(ts)
  {}

  void processAnswer(Serializer::SerializedContainer answer) {
    const message_t* ms = deserialzeMessage(answer);
    this->ts_.setValue(ms->ticket_, ms->payload_);
  }

private:
  const message_t* const deserialzeMessage(/*const*/ Serializer::SerializedContainer& answer) const {
    return Serializer::template deserialize<message_t>(answer);
  }

  ticket_store<typename Serializer::SerializedContainer>& ts_;
};