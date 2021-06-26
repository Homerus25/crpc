#pragma once

#include "rpc_async_client.h"


#define MQTT_STD_VARIANT
#define MQTT_STD_OPTIONAL
//#define MQTT_USE_LOG
//#define BOOST_LOG_DYN_LINK

#include "message.h"
#include "mqtt_cpp/include/mqtt_client_cpp.hpp"
#include "mqtt_cpp/include/mqtt/setup_log.hpp"

#include "ticket_store.h"

struct rpc_mqtt_transport {

    explicit rpc_mqtt_transport(boost::asio::io_context &ioc, std::string const& name, std::uint16_t port)
    : ioc_(ioc)
    {
      mqtt_client = mqtt::make_client(ioc, name, port);
      //mqtt_client->set_client_id("cid1");
      mqtt_client->set_clean_session(true);
      mqtt_client->set_connack_handler(
          [&](bool sp, mqtt::connect_return_code connack_return_code) {
            if (connack_return_code == mqtt::connect_return_code::accepted) {
              //mqtt_client->async_subscribe("topic1", mqtt::qos::at_most_once);
              mqtt_client->subscribe("topic1", mqtt::qos::at_most_once);
              //std::cout << "subscribed!" << std::endl;
            }
            return true;
          });

      mqtt_client->set_close_handler([] {
        //std::cout << "closed!" << std::endl;
      });
      mqtt_client->set_error_handler(
          [](boost::system::error_code const& ec) {
            std::cerr << "error occured: " << ec << std::endl;
          });
      mqtt_client->set_puback_handler(
          [](std::uint16_t packet_id) { return true; });
      mqtt_client->set_pubrec_handler(
          [](std::uint16_t packet_id) { return true; });
      mqtt_client->set_pubcomp_handler(
          [](std::uint16_t packet_id) { return true; });

      mqtt_client->set_suback_handler(
          [&](std::uint16_t packet_id,
             std::vector<mqtt::suback_return_code> results) {
            return true;
          });

      mqtt_client->set_publish_handler(
          [&](
              mqtt::optional<std::uint16_t> packet_id,
              mqtt::publish_options options, mqtt::buffer topic_name,
              mqtt::buffer contents) {
            //std::cout << "pub handler, topic: " << topic_name
            //          << " content: " << contents << std::endl; //"\n";

            auto ms = cista::deserialize<message>(contents);
            ts_.setValue(ms->ticket_, ms->payload_);

            return true;
          });
      mqtt_client->connect();
  }

  std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                               std::vector<unsigned char> const& params) {
    message ms{
        ts_.nextNumber(), fn_idx,
        cista::offset::vector<unsigned char>(params.begin(), params.end())};
    auto future = ts_.emplace(ms.ticket_);

    auto const ms_buf = cista::serialize(ms);
    auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
    mqtt_client->publish("topic1", ms_string);  //, mqtt::qos::at_most_once);
    //mqtt_client->async_publish(MQTT_NS::allocate_buffer("rpc"), MQTT_NS::allocate_buffer(ms_string), MQTT_NS::qos::exactly_once,[](MQTT_NS::error_code){});
    //mqtt_client->async_publish("rpc", ms_string, MQTT_NS::qos::exactly_once,[](MQTT_NS::error_code){});
    //mqtt_client->async_publish("rpc", "test", MQTT_NS::qos::exactly_once,[](MQTT_NS::error_code){});

    return future;
  }

  ~rpc_mqtt_transport() {
    mqtt_client->unsubscribe("topic1");
    mqtt_client->disconnect(std::chrono::seconds(1));
  }

private:
  std::shared_ptr<mqtt::callable_overlay<mqtt::client<mqtt::tcp_endpoint<as::ip::tcp::socket, as::io_context::strand>>>> mqtt_client;
  ticket_store ts_;
  boost::asio::io_context &ioc_;
};

template<typename Interface>
using rpc_mqtt_client = rpc_async_client<rpc_mqtt_transport, Interface>;
