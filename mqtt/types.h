#ifndef MQTT_TYPES_H_
#define MQTT_TYPES_H_

#include <cstdint>
#include <string_view>
#include <vector>

namespace mqtt {

using MqttBytes = std::vector<uint8_t>;

struct Topic {
   std::string_view value;
};

} // namespace mqtt

#endif // MQTT_TYPES_H_
