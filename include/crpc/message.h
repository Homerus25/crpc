#pragma once

#include <cstdint>

template<typename SerializedContainer>
struct message {
    uint64_t ticket_;
    unsigned fn_idx;
    SerializedContainer payload_;
};

