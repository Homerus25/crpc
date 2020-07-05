#pragma once

#include "cista/containers/vector.h"

/*
struct message {
   unsigned fn_idx;
   cista::offset::vector<unsigned char> payload_;
};
*/

struct message {
    uint64_t ticket_;
    unsigned fn_idx;
    cista::offset::vector<unsigned char> payload_;
};

