#pragma once

#include "rpc_server.h"
#include "message.h"
#include "deserialize_helper.h"

#define MQTT_STD_VARIANT
#define MQTT_STD_OPTIONAL
//#define MQTT_USE_LOG

#include "mqtt_cpp/include/mqtt_server_cpp.hpp"
#include "mqtt_cpp/include/mqtt/setup_log.hpp"

#include <boost/asio.hpp>

#include <iostream>

template <typename Interface>
class rpc_mqtt_server : public rpc_server<Interface> {
public:
    explicit rpc_mqtt_server(std::uint16_t port)
        : port_(std::move(port))
    {}

    void run();

private:
    std::uint16_t port_;
};


namespace mi = boost::multi_index;

using con_t = MQTT_NS::server<>::endpoint_t;
using con_sp_t = std::shared_ptr<con_t>;

struct sub_con {
  sub_con(MQTT_NS::buffer topic, con_sp_t con, MQTT_NS::qos qos_value)
      :topic(std::move(topic)), con(std::move(con)), qos_value(qos_value) {}
  MQTT_NS::buffer topic;
  con_sp_t con;
  MQTT_NS::qos qos_value;
};

struct tag_topic {};
struct tag_con {};

using mi_sub_con = mi::multi_index_container<
sub_con,
mi::indexed_by<
    mi::ordered_non_unique<
    mi::tag<tag_topic>,
BOOST_MULTI_INDEX_MEMBER(sub_con, MQTT_NS::buffer, topic)
>,
mi::ordered_non_unique<
    mi::tag<tag_con>,
BOOST_MULTI_INDEX_MEMBER(sub_con, con_sp_t, con)
>
>
>;

inline void close_proc(std::set<con_sp_t>& cons, mi_sub_con& subs, con_sp_t const& con) {
  cons.erase(con);

  auto& idx = subs.get<tag_con>();
  auto r = idx.equal_range(con);
  idx.erase(r.first, r.second);
}


template<typename Interface>
void rpc_mqtt_server<Interface>::run()
{
  boost::asio::io_context ioc;

  auto server = MQTT_NS::server<>(
      boost::asio::ip::tcp::endpoint(
          boost::asio::ip::tcp::v4(),
          port_
      ),
      ioc
  );

  server.set_error_handler(
      [](MQTT_NS::error_code ec) {
        std::cout << "error: " << ec.message() << std::endl;
      }
  );

  std::set<con_sp_t> connections;
  mi_sub_con subs;

  server.set_accept_handler(
      [&connections, &subs, this](con_sp_t spep) {
        auto& ep = *spep;
        std::weak_ptr<con_t> wp(spep);

        using packet_id_t = typename std::remove_reference_t<decltype(ep)>::packet_id_t;
        //std::cout << "accept" << std::endl;

        // Pass spep to keep lifetime.
        // It makes sure wp.lock() never return nullptr in the handlers below
        // including close_handler and error_handler.
        ep.start_session(std::move(spep));

        ep.set_disconnect_handler([](){
          //std::cout << "disconneted!" << std::endl;
         });
        // set connection (lower than MQTT) level handlers
        ep.set_close_handler(
            [&connections, &subs, wp]
                (){
              //std::cout << "[server] closed." << std::endl;
              auto sp = wp.lock();
              BOOST_ASSERT(sp);
              close_proc(connections, subs, sp);
            });
        ep.set_error_handler(
            [&connections, &subs, wp]
                (MQTT_NS::error_code ec){
              std::cerr << "[server] error: " << ec.message() << std::endl;
              auto sp = wp.lock();
              BOOST_ASSERT(sp);
              close_proc(connections, subs, sp);
            });

        // set MQTT level handlers
        ep.set_connect_handler(
            [&connections, wp]
                (MQTT_NS::buffer client_id,
                 MQTT_NS::optional<MQTT_NS::buffer> username,
                 MQTT_NS::optional<MQTT_NS::buffer> password,
                 MQTT_NS::optional<MQTT_NS::will>,
                 bool clean_session,
                 std::uint16_t keep_alive) {
              using namespace MQTT_NS::literals;
              /*
              std::cout << "[server] client_id    : " << client_id << std::endl;
              std::cout << "[server] username     : " << (username ? username.value() : "none"_mb) << std::endl;
              std::cout << "[server] password     : " << (password ? password.value() : "none"_mb) << std::endl;
              std::cout << "[server] clean_session: " << std::boolalpha << clean_session << std::endl;
              std::cout << "[server] keep_alive   : " << keep_alive << std::endl;
               */
              //std::cout << "client connected!" << std::endl;
              auto sp = wp.lock();
              BOOST_ASSERT(sp);
              connections.insert(sp);
              sp->connack(false, MQTT_NS::connect_return_code::accepted);
              return true;
            }
        );
        ep.set_publish_handler(
            [this, &subs, wp]
                (MQTT_NS::optional<packet_id_t> packet_id,
                 MQTT_NS::publish_options pubopts,
                 MQTT_NS::buffer topic_name,
                 MQTT_NS::buffer contents){
              /*
              std::cout << "[server] publish received."
                        << " dup: "    << pubopts.get_dup()
                        << " qos: "    << pubopts.get_qos()
                        << " retain: " << pubopts.get_retain() << std::endl;
              if (packet_id)
                std::cout << "[server] packet_id: " << *packet_id << std::endl;
              std::cout << "[server] topic_name: " << topic_name << std::endl;
              std::cout << "[server] contents: " << contents << std::endl;
              */

              //std::cout << "published!" << std::endl;

              auto const req = cista::deserialize<message>(contents);

              auto const func_num = req->fn_idx;
              if (func_num < this->fn_.size()) {
                //std::cout << "passed with function number: " << func_num << "!\n";
                //std::cout << "size: " << this->fn_.size() << "\n";

                auto const pload = this->call(func_num,
                                              std::vector<unsigned char>(req->payload_.begin(),
                                                                         req->payload_.end()));
                message ms{req->ticket_, func_num,
                           cista::offset::vector<unsigned char>(pload.begin(), pload.end())};

                auto const res_buf = cista::serialize(ms);

                auto sp = wp.lock();
                BOOST_ASSERT(sp);
                sp->async_publish("topic1", std::string(begin(res_buf), end(res_buf)));
                //sp->async_publish("crpc1", std::string(begin(res_buf), end(res_buf)));
                //sp->async_publish(topic_name.to_string(), std::string(begin(res_buf), end(res_buf)));
              }


              return true;
            });
        ep.set_subscribe_handler(
            [&subs, wp]
                (packet_id_t packet_id,
                 std::vector<MQTT_NS::subscribe_entry> entries) {
              //std::cout << "[server]subscribe received. packet_id: " << packet_id << std::endl;
              std::vector<MQTT_NS::suback_return_code> res;
              res.reserve(entries.size());
              auto sp = wp.lock();
              BOOST_ASSERT(sp);
              /*
              for (auto const& e : entries) {
               //std::cout << "[server] topic_filter: " << e.topic_filter  << " qos: " << e.subopts.get_qos() << std::endl;
                res.emplace_back(MQTT_NS::qos_to_suback_return_code(e.subopts.get_qos()));
                subs.emplace(std::move(e.topic_filter), sp, e.subopts.get_qos());
                //subs.emplace(e.topic_filter, sp, e.subopts.get_qos());
              }
               */
              sp->suback(packet_id, res);
              return true;
            }
        );
        ep.set_unsubscribe_handler(
            [&subs, wp]
                (packet_id_t packet_id,
                 std::vector<MQTT_NS::unsubscribe_entry> entries) {
              //std::cout << "[server]unsubscribe received. packet_id: " << packet_id << std::endl;
              for (auto const& e : entries) {
                subs.erase(e.topic_filter);
              }
              auto sp = wp.lock();
              BOOST_ASSERT(sp);
              sp->unsuback(packet_id);
              return true;
            }
        );
      }
  );

  server.listen();

  std::thread t1([&](){ ioc.run(); });
  std::thread t2([&](){ ioc.run(); });

  ioc.run();
  t1.join();
  t2.join();
}
