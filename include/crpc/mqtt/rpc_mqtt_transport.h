#pragma once

#define MQTT_STD_VARIANT
#define MQTT_STD_OPTIONAL
//#define MQTT_USE_LOG
//#define BOOST_LOG_DYN_LINK

#include <mqtt_client_cpp.hpp>

#include "../log.h"
#include "../receiver.h"

struct rpc_mqtt_transport {
    explicit rpc_mqtt_transport(Receiver rec, std::string const& name = "127.0.0.1", std::uint16_t port = 2000)
      : receiver(rec)
    {
      mqtt_client_ = mqtt::make_client(ioc_, name, port, mqtt::protocol_version::v5);

      mqtt_client_->set_client_id("cid1");
      mqtt_client_->set_clean_start(true);

      set_handler();

      boost::system::error_code ec;
      mqtt_client_->connect(ec);
      if(ec)
        std::cout << "error connecting: " << ec.what() << std::endl;

      runner = std::thread([&](){ ioc_.run(); });

      while(!mqtt_client_->connected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
  }

  void send(std::vector<unsigned char> ms_buf) {
      mqtt_client_->publish(0, MQTT_NS::allocate_buffer(topic), MQTT_NS::allocate_buffer(ms_buf.begin(), ms_buf.end()));
  }

  void close_connection() {
    mqtt_client_->disconnect();
  }

  void stop() {
    close_connection();
    runner.join();
  }

private:
  Receiver receiver;
  const std::string topic = "mqtt_client_cpp/topic1";

  using MQTT_CO = mqtt::callable_overlay<mqtt::client<mqtt::tcp_endpoint<boost::asio::ip::tcp::socket, mqtt::strand>>>;
  using packet_id_t = typename std::remove_reference_t<MQTT_CO>::packet_id_t;

  boost::asio::io_context ioc_;
  std::thread runner;
  std::shared_ptr<MQTT_CO> mqtt_client_;

  void set_handler() {
    mqtt_client_->set_v5_connack_handler( get_connack_handler());
    mqtt_client_->set_close_handler([&] {
      Log("closed!");
    });
    mqtt_client_->set_error_handler(
        [](MQTT_NS::error_code const& ec) {
          LogErr("[client] error occured: " , ec);
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
