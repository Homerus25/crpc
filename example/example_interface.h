#pragma once

#include "crpc/rpc_client.h"

struct example_interface {
    fn<int, int, int> add_;
    fn<void> hello_world_;
    fn<void, int> inc_count_;
    fn<int> get_count_;
};
