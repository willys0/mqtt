#ifndef MQTT_PACKET_PACKET_H_
#define MQTT_PACKET_PACKET_H_

#include "mqtt/types.h"

namespace mqtt::packet {

struct ConnectOptions {
   std::string_view client_id;
   uint16_t keep_alive_seconds{0U};
   bool clean_session{true};
};

MqttBytes encodeConnect(ConnectOptions const &connect_options);
MqttBytes encodePublish(PublishMessage const &publish_message);

} // namespace mqtt::packet

#endif // MQTT_PACKET_PACKET_H_
