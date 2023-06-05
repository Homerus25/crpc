#pragma once

#include "../rpc_server.h"
#include "../message.h"
#include "../deserialize_helper.h"

#define MQTT_STD_VARIANT
#define MQTT_STD_OPTIONAL
//#define MQTT_USE_LOG

#include <mqtt_server_cpp.hpp>
//#include "mqtt_cpp/include/mqtt/setup_log.hpp"

#include "mqtt_utils.h"


template <typename Interface>
class rpc_mqtt_server : public rpc_server<Interface> {
public:
  explicit rpc_mqtt_server(std::uint16_t port)
      : port_(port),
        server(MQTT_NS::server<>(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port_),ioc))
  {
    set_handler(server);
    server.listen();
  }

  void run(int threads_count);
  void stop() {
    running = false;
    server.close();
    for(auto& t: runner)
      t.join();
  }
  void block() {
    ioc.run();
  }

private:
  using MQTT_CO = MQTT_NS::callable_overlay<MQTT_NS::server_endpoint<std::mutex, std::lock_guard,2>>;
  using packet_id_t = typename std::remove_reference_t<MQTT_CO>::packet_id_t;

  MQTT_CO::v5_disconnect_handler get_disconnect_handler(std::weak_ptr<con_t>&);
  MQTT_CO::close_handler get_close_handler(std::weak_ptr<con_t>&);
  MQTT_CO::error_handler get_error_handler(std::weak_ptr<con_t>&);
  MQTT_CO::v5_connect_handler get_connect_handler(std::weak_ptr<con_t>&);
  MQTT_CO::v5_publish_handler get_publish_handler(std::weak_ptr<con_t>&);
  MQTT_CO::v5_subscribe_handler get_subscribe_handler(std::weak_ptr<con_t>&);
  MQTT_CO::v5_unsubscribe_handler get_unsubscribe_handler(std::weak_ptr<con_t>&);
  void set_handler(mqtt::server<mqtt::strand, std::mutex, std::lock_guard, 2>& server);

  std::set<con_sp_t> connections_;
  mi_sub_con subs_;
  std::uint16_t port_;
  boost::asio::io_context ioc;
  MQTT_NS::server<> server;
  std::vector<std::thread> runner;
  bool running{false};
};

#include "mqtt_server_handler.h"

template<typename Interface>
void rpc_mqtt_server<Interface>::run(int threads_count)
{
  running = true;
  for(int i=0; i<threads_count; ++i)
    runner.emplace_back([&](){ ioc.run(); });
}

template<typename Interface>
void rpc_mqtt_server<Interface>::set_handler(
    mqtt::server<mqtt::strand, std::mutex, std::lock_guard, 2>& server) {

  server.set_error_handler(
      [&](MQTT_NS::error_code error_code) {
        if (running)
          LogErr("error: ", error_code.message());
      }
  );

  server.set_accept_handler(
      [this](con_sp_t spep) {

        auto& endpoint = *spep;
        std::weak_ptr<con_t> wp(spep);

        Log("accept");

        endpoint.start_session(std::move(spep));
        endpoint.set_v5_disconnect_handler(this->get_disconnect_handler(wp));
        endpoint.set_close_handler(this->get_close_handler(wp));
        endpoint.set_error_handler(this->get_error_handler(wp));

        // set MQTT level handlers
        endpoint.set_v5_connect_handler(this->get_connect_handler(wp));
        endpoint.set_v5_publish_handler(this->get_publish_handler(wp));
        endpoint.set_v5_subscribe_handler(this->get_subscribe_handler(wp));
        endpoint.set_v5_unsubscribe_handler(this->get_unsubscribe_handler(wp));
        endpoint.set_v5_unsuback_handler([](packet_id_t,
                                              std::vector<MQTT_NS::v5::unsuback_reason_code> reasons,
                                            MQTT_NS::v5::properties props){return true;});
        endpoint.set_v5_connack_handler([](bool session_present,
                                           MQTT_NS::v5::connect_reason_code reason_code,
                                           MQTT_NS::v5::properties props){return true;});
      }
  );
}
