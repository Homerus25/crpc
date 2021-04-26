#pragma once

#include "rpc_server.h"
#include "rpc_client.h"

template <typename Interface>
struct no_network_transport {
    explicit no_network_transport(rpc_server<Interface>& s) : s_{s} {}

    std::vector<unsigned char> send(unsigned fn_idx,
                                    std::vector<unsigned char> const& params) {
        return s_.call(fn_idx, params);
    }

    rpc_server<Interface>& s_;
};

template <typename Interface>
using no_network_client = rpc_client<no_network_transport<Interface>, Interface>;

