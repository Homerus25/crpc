#pragma once


rpc_mqtt_transport::MQTT_CO::v5_suback_handler
rpc_mqtt_transport::get_suback_handler() {
  return [&]
      (packet_id_t packet_id,
       const std::vector<MQTT_NS::v5::suback_reason_code>& reasons,
       const MQTT_NS::v5::properties& /*props*/){
        Log("[client] suback received. packet_id: " , packet_id);
        for (auto const& e : reasons) {
          switch (e) {
            case MQTT_NS::v5::suback_reason_code::granted_qos_0:
              Log("[client] subscribe success: qos0");
              break;
            case MQTT_NS::v5::suback_reason_code::granted_qos_1:
              Log("[client] subscribe success: qos1");
              break;
            case MQTT_NS::v5::suback_reason_code::granted_qos_2:
              Log("[client] subscribe success: qos2");
              break;
            default:
              Log("[client] subscribe failed: reason_code = " , static_cast<int>(e));
              break;
          }
        }
        return true;
      };
}

rpc_mqtt_transport::MQTT_CO::v5_pubcomp_handler
rpc_mqtt_transport::get_pubcomp_handler() const {
  return [&]
      (packet_id_t packet_id, MQTT_NS::v5::pubcomp_reason_code reason_code, const MQTT_NS::v5::properties& /*props*/){
        Log(
            "[client] pubcomp received. packet_id: " , packet_id ,
            " reason_code: " , reason_code);
        return true;
      };
}

rpc_mqtt_transport::MQTT_CO::v5_publish_handler
rpc_mqtt_transport::get_publish_handler() {
  return [&]
      (std::optional<packet_id_t> packet_id,
       MQTT_NS::publish_options pubopts,
       MQTT_NS::buffer topic_name,
       MQTT_NS::buffer contents,
       const MQTT_NS::v5::properties& /*props*/){
        Log("[client] publish received. "
            , "dup: "     , pubopts.get_dup()
                         , " qos: "    , pubopts.get_qos()
                          , " retain: " , pubopts.get_retain());
        if (packet_id)
          Log("[client] packet_id: " , *packet_id);
        Log("[client] topic_name: " , topic_name);
        Log("[client] contents: " , contents);

        std::vector<unsigned char> dd(contents.begin(), contents.end());
        auto ms = cista::deserialize<message>(dd);
        this->ts_.setValue(ms->ticket_, ms->payload_);

        return true;
      };
}

rpc_mqtt_transport::MQTT_CO::v5_pubrec_handler
rpc_mqtt_transport::get_pubrec_handler() const {
  return [&]
      (packet_id_t packet_id, MQTT_NS::v5::pubrec_reason_code reason_code, const MQTT_NS::v5::properties& /*props*/){
        Log(
            "[client] pubrec received. packet_id: " , packet_id ,
            " reason_code: " , reason_code);
        return true;
      };
}

rpc_mqtt_transport::MQTT_CO::v5_puback_handler
rpc_mqtt_transport::get_puback_handler() const {
  return [&]
      (packet_id_t packet_id, MQTT_NS::v5::puback_reason_code reason_code, const MQTT_NS::v5::properties& /*props*/){
        Log(
            "[client] puback received. packet_id: " , packet_id ,
            " reason_code: " , reason_code);
        return true;
      };
}

rpc_mqtt_transport::MQTT_CO::v5_connack_handler
rpc_mqtt_transport::get_connack_handler() {
  return [&](bool sp, MQTT_NS::v5::connect_reason_code reason_code, MQTT_NS::v5::properties /*props*/) {
    Log("[client] Connack handler called");
    Log("[client] Session Present: " , std::boolalpha , sp);
    Log("[client] Connect Reason Code: " , reason_code);
    if (reason_code == MQTT_NS::v5::connect_reason_code::success) {
      mqtt_client_->subscribe(topic, MQTT_NS::qos::at_most_once);
    }
    return true;
  };
}