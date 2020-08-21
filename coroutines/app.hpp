#pragma once

#include <boost/asio/signal_set.hpp>
#include <boost/asio/basic_signal_set.hpp>

#include "config.hpp"
#include "server.hpp"

#include "../example/example_interface.h"

#include <boost/asio.hpp>

namespace project {
    /// The application object.
    /// There shall be one.
    /// So no need to be owned by a shared ptr
    struct app {
        //app(net::executor exec);
        app(net::io_context&);

        void run();

    private:

        void handle_run();

        // The application's executor.
        // In a multi-threaded application this would be a strand.
        net::executor exec_;
        net::signal_set signals_;
        server<example_interface> server_;
    };
}
