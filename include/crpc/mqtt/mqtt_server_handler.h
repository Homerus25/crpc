#pragma once

#include "../log.h"

template <typename Interface>
rpc_mqtt_server<Interface>::MQTT_CO::v5_unsubscribe_handler
rpc_mqtt_server<Interface>::get_unsubscribe_handler(std::weak_ptr<con_t>& wp) {
  return [this, wp]
      (packet_id_t packet_id,
       std::vector<MQTT_NS::unsubscribe_entry> entries,
       const MQTT_NS::v5::properties& /*props*/) {
        Log("[server]unsubscribe received. packet_id: ", packet_id);
        auto sp = wp.lock();
        for (auto const& e : entries) {
          auto it = subs_.find(std::make_tuple(sp, e.topic_filter));
          if (it != subs_.end()) {
            subs_.erase(it);
          }
        }
        BOOST_ASSERT(sp);
        //sp->unsuback(packet_id);
        return true;
      };
}
template <typename Interface>
rpc_mqtt_server<Interface>::MQTT_CO::v5_subscribe_handler
rpc_mqtt_server<Interface>::get_subscribe_handler(std::weak_ptr<con_t>& wp) {
  return [this, wp]
      (packet_id_t packet_id,
       std::vector<MQTT_NS::subscribe_entry> entries,
       const MQTT_NS::v5::properties& /*props*/) {
        Log("[server]subscribe received. packet_id: ", packet_id);
        std::vector<MQTT_NS::v5::suback_reason_code> res;
        res.reserve(entries.size());
        auto sp = wp.lock();
        BOOST_ASSERT(sp);

        for (auto const& entry : entries) {
          //std::cout << "[server] topic_filter: " << entry.topic_filter  << " qos: " << entry.subopts.get_qos() << std::endl;
          res.emplace_back(MQTT_NS::v5::qos_to_suback_reason_code(
              entry.subopts.get_qos()));
          //subs.emplace(std::move(entry.topic_filter), sp, entry.subopts.get_qos());
          subs_.emplace(std::move(entry.topic_filter), sp,
                        entry.subopts.get_qos(), entry.subopts.get_rap());
          //subs.emplace(entry.topic_filter, sp, entry.subopts.get_qos());
        }

        sp->suback(packet_id, res);
        return true;
      };
}
template <typename Interface>
rpc_mqtt_server<Interface>::MQTT_CO::v5_publish_handler
rpc_mqtt_server<Interface>::get_publish_handler(std::weak_ptr<con_t>& wp) {
  return [this, wp]
      (MQTT_NS::optional<packet_id_t> packet_id,
       MQTT_NS::publish_options pubopts,
       const MQTT_NS::buffer& topic_name,
       MQTT_NS::buffer contents,
       const MQTT_NS::v5::properties& props){

        Log("[server] publish received."
                  , " dup: "    , pubopts.get_dup()
                  , " qos: "    , pubopts.get_qos()
                  , " retain: " , pubopts.get_retain());
        if (packet_id)
          Log("[server] packet_id: " , *packet_id);
        Log("[server] topic_name: " , topic_name);
        Log("[server] contents: " , contents);

        /*
        auto const& idx = subs.get<tag_topic>();
        auto r = idx.equal_range(topic_name);
        for (; r.first != r.second; ++r.first) {
          auto retain = [&] {
            if (r.first->rap_value == MQTT_NS::rap::retain) {
              return pubopts.get_retain();
            }
            return MQTT_NS::retain::no;
          }();
          r.first->con->publish(
              topic_name, contents,
              std::min(r.first->qos_value, pubopts.get_qos()) | retain,
              std::move(props));
        }
        */

        //auto const response = this->template process_message(contents);
        //auto x = contents.data()
        std::vector<unsigned char> dd(contents.begin(), contents.end());
        //auto const response = this->process_message(contents.data());
        auto const response = this->process_message(dd);
        if(response) {
          auto shared_ptr_endpoint = wp.lock();
          BOOST_ASSERT(shared_ptr_endpoint);
          auto res_buf = response.value();
          shared_ptr_endpoint->async_publish("topic1", std::string(begin(res_buf), end(res_buf)));
        }


        return true;
      };
}

template <typename Interface>
rpc_mqtt_server<Interface>::MQTT_CO::v5_connect_handler
rpc_mqtt_server<Interface>::get_connect_handler(std::weak_ptr<con_t>& wp) {
  return [this, wp]
      (MQTT_NS::buffer client_id,
       MQTT_NS::optional<MQTT_NS::buffer> username,
       MQTT_NS::optional<MQTT_NS::buffer> password,
       MQTT_NS::optional<MQTT_NS::will>,
       bool clean_session,
       std::uint16_t keep_alive,
       const MQTT_NS::v5::properties& /*props*/) {
        using namespace MQTT_NS::literals;
        Log("[server] client_id    : ",client_id);
        //std::cout << "[server] client_id    : " << client_id << std::endl;
        Log("[server] username     : " , (username.has_value() ? username.value().to_string() : "none"));
        Log("[server] password     : " , (password.has_value() ? password.value().to_string() : "none"));
        Log("[server] clean_session: " , std::boolalpha , clean_session);
        Log("[server] keep_alive   : " , keep_alive);

        //std::cout << "client connected!" << std::endl;
        auto sp = wp.lock();
        BOOST_ASSERT(sp);
        connections_.insert(sp);
        //sp->connack(false, MQTT_NS::connect_return_code::accepted);
        sp->connack(false, MQTT_NS::v5::connect_reason_code::success);

        return true;
      };
}
template <typename Interface>
rpc_mqtt_server<Interface>::MQTT_CO::error_handler
rpc_mqtt_server<Interface>::get_error_handler(std::weak_ptr<con_t>& wp) {
  return [this, wp]
      (MQTT_NS::error_code ec){
        LogErr("[server] error: ", ec.message());
        auto sp = wp.lock();
        BOOST_ASSERT(sp);
        close_proc(connections_, subs_, sp);
      };
}

template <typename Interface>
rpc_mqtt_server<Interface>::MQTT_CO::v5_disconnect_handler
rpc_mqtt_server<Interface>::get_disconnect_handler(std::weak_ptr<con_t>& wp) {
  return [this, wp](MQTT_NS::v5::disconnect_reason_code reason_code, const MQTT_NS::v5::properties& /*props*/){
    Log("[server] disconnect received.");
    auto sp = wp.lock();
    BOOST_ASSERT(sp);
    close_proc(connections_, subs_, sp);
  };
}

template <typename Interface>
rpc_mqtt_server<Interface>::MQTT_CO::close_handler
rpc_mqtt_server<Interface>::get_close_handler(std::weak_ptr<con_t>& wp) {
  return [this, wp](){
    Log("[server] closed.");
    auto sp = wp.lock();
    BOOST_ASSERT(sp);
    close_proc(connections_, subs_, sp);
  };
}
