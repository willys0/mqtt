#ifndef MQTT_TYPES_H_
#define MQTT_TYPES_H_

#include <cstdint>
#include <string_view>
#include <vector>

namespace mqtt {

using MqttBytes = std::vector<uint8_t>;

enum class QoS : uint8_t {
   kAtMostOnce = 0,
   kAtLeastOnce = 1,
   kExactlyOnce = 2
};

struct Topic {
   std::string_view value;
};

struct PublishMessage {
   Topic topic;
   std::string_view message;
   QoS qos{QoS::kAtMostOnce};
   bool retain{false};
};

} // namespace mqtt

#endif // MQTT_TYPES_H_
