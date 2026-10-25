#ifndef MQTT_TRANSPORT_H_
#define MQTT_TRANSPORT_H_

#include <optional>

#include "mqtt/types.h"

namespace mqtt {
class ITransport {
 public:
   virtual ~ITransport() = default;

   virtual bool sendBytes(MqttBytes const &bytes) = 0;
   virtual std::optional<MqttBytes> receiveBytes() = 0;

 protected:
   ITransport() = default;
   ITransport(const ITransport &) = default;
   ITransport(ITransport &&) = default;
   ITransport &operator=(const ITransport &) = default;
   ITransport &operator=(ITransport &&) = default;
};

} // namespace mqtt

#endif // MQTT_TRANSPORT_H_
