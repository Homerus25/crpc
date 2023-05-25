#pragma once

namespace mi = boost::multi_index;

using con_t = MQTT_NS::server<>::endpoint_t;
using con_sp_t = std::shared_ptr<con_t>;

struct sub_con {
  sub_con(MQTT_NS::buffer topic, con_sp_t con, MQTT_NS::qos qos_value, MQTT_NS::rap rap_value)
      :topic(std::move(topic)), con(std::move(con)), qos_value(qos_value), rap_value(rap_value) {}
  MQTT_NS::buffer topic;
  con_sp_t con;
  MQTT_NS::qos qos_value;
  MQTT_NS::rap rap_value;
};

struct tag_topic {};
struct tag_con {};
struct tag_con_topic {};

using mi_sub_con = mi::multi_index_container<
    sub_con,
    mi::indexed_by<
        mi::ordered_unique<
            mi::tag<tag_con_topic>,
            mi::composite_key<
                sub_con,
                BOOST_MULTI_INDEX_MEMBER(sub_con, con_sp_t, con),
                BOOST_MULTI_INDEX_MEMBER(sub_con, MQTT_NS::buffer, topic)
                >
            >,
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
  con->disconnect();
  cons.erase(con);

  auto& idx = subs.get<tag_con>();
  auto r = idx.equal_range(con);
  idx.erase(r.first, r.second);
}