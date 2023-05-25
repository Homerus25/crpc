#pragma once

#include "../rpc_async_client.h"

#define MQTT_STD_VARIANT
#define MQTT_STD_OPTIONAL
//#define MQTT_USE_LOG
//#define BOOST_LOG_DYN_LINK

#include "../message.h"
#include <mqtt_client_cpp.hpp>
//#include <setup_log.hpp>

#include "../ticket_store.h"

#include "../log.h"

struct rpc_mqtt_transport {
    explicit rpc_mqtt_transport( std::string const& name, std::uint16_t port)
    {
      mqtt_client_ = mqtt::make_client(ioc_, name, port, mqtt::protocol_version::v5);

      mqtt_client_->set_client_id("cid1");
      mqtt_client_->set_clean_start(true);

      set_handler();

      mqtt_client_->connect();
      runner = std::thread([&](){ ioc_.run(); });
  }

  std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                               std::vector<unsigned char> const& params) {
    message ms{
        ts_.nextNumber(), fn_idx,
        cista::offset::vector<unsigned char>(params.begin(), params.end())};
    auto future = ts_.emplace(ms.ticket_);

    auto const ms_buf = cista::serialize(ms);
    auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
    mqtt_client_->async_publish(topic, ms_string);

    return future;
  }

  void close_connection() {
    mqtt_client_->disconnect();
  }

  void stop() {
    close_connection();
    runner.join();
  }

private:

  const std::string topic = "mqtt_client_cpp/topic1";

  using MQTT_CO = mqtt::callable_overlay<mqtt::client<mqtt::tcp_endpoint<boost::asio::ip::tcp::socket, mqtt::strand>>>;
  using packet_id_t = typename std::remove_reference_t<MQTT_CO>::packet_id_t;
  std::shared_ptr<MQTT_CO> mqtt_client_;
  ticket_store ts_;
  boost::asio::io_context ioc_;
  std::thread runner;

  void set_handler() {
    mqtt_client_->set_v5_connack_handler( get_connack_handler());
    mqtt_client_->set_close_handler([&] {
      Log("closed!");
    });
    mqtt_client_->set_error_handler(
        [](MQTT_NS::error_code const& ec) {
          LogErr("error occured: " , ec);
        });
    mqtt_client_->set_v5_puback_handler(get_puback_handler());
    mqtt_client_->set_v5_pubrec_handler(get_pubrec_handler());
    mqtt_client_->set_v5_pubcomp_handler(get_pubcomp_handler());

    mqtt_client_->set_v5_suback_handler(get_suback_handler());
    mqtt_client_->set_v5_publish_handler(get_publish_handler());
    mqtt_client_->set_v5_disconnect_handler([](MQTT_NS::v5::disconnect_reason_code reason_code, MQTT_NS::v5::properties props) {
      Log("disconnected");
    });
  }

  MQTT_CO::v5_connack_handler get_connack_handler();
  MQTT_CO::v5_puback_handler get_puback_handler() const;
  MQTT_CO::v5_pubrec_handler get_pubrec_handler() const;
  MQTT_CO::v5_pubcomp_handler get_pubcomp_handler() const;
  static MQTT_CO::v5_suback_handler get_suback_handler();
  MQTT_CO::v5_publish_handler get_publish_handler();
};// __attribute__((aligned(128))) __attribute__((packed));

#include "mqtt_client_handler.h"

template<typename Interface>
using rpc_mqtt_client = rpc_async_client<rpc_mqtt_transport, Interface>;
