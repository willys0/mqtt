#ifndef MQTT_PACKET_PACKET_H_
#define MQTT_PACKET_PACKET_H_

#include "mqtt/types.h"

namespace mqtt::packet {

MqttBytes encodeConnect();
MqttBytes encodePublish(PublishMessage const &publish_message);

} // namespace mqtt::packet

#endif // MQTT_PACKET_PACKET_H_
