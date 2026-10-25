#ifndef _MQTT_CLIENT_MQTT_H_
#define _MQTT_CLIENT_MQTT_H_

#include "mqtt/transport.h"
#include "mqtt/types.h"

namespace mqtt {

bool mqttConnect(ITransport &transport);
bool mqttPublish(ITransport &transport, PublishMessage const& publish_message);

} // namespace mqtt

#endif // _MQTT_CLIENT_MQTT_H_
