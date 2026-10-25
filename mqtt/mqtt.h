#ifndef _MQTT_CLIENT_MQTT_H_
#define _MQTT_CLIENT_MQTT_H_

#include "mqtt/transport.h"
#include <string_view>

namespace mqtt {

bool mqttConnect(ITransport &transport);
bool mqttPublish(ITransport &transport, Topic topic,
                 std::string_view msg);

} // namespace mqtt

#endif // _MQTT_CLIENT_MQTT_H_
