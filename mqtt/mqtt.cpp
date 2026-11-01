#include "mqtt/mqtt.h"
#include "mqtt/packet/packet.h"
#include "mqtt/transport.h"
#include "mqtt/types.h"
#include <cassert>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include <span>

namespace mqtt {

namespace {
enum class MqttControlPackageType : uint8_t {
   kInvalid = 0,
   kConnect = 1 << 4,
   kConnAck = 2 << 4,
   kPublish = 3 << 4,
   kPubAck = 4 << 4,
   kSubAck = 9 << 4
};

struct MqttResponse {
   MqttControlPackageType type{};
};

bool isValidResponse(MqttControlPackageType const type) {
   switch (type) {
   case MqttControlPackageType::kConnAck:
   case MqttControlPackageType::kSubAck:
      return true;
   default:
      return false;
   }
}

std::optional<MqttControlPackageType>
tryParsePackageType(std::span<uint8_t const> const bytes) {
   uint8_t package_type_byte{bytes[0]};

   MqttControlPackageType type{package_type_byte};

   if (isValidResponse(type)) {
      return type;
   }
   return std::nullopt;
}

std::optional<MqttResponse>
tryParseResponse(std::span<uint8_t const> const bytes) {
   return tryParsePackageType(bytes).and_then(
       [](MqttControlPackageType const type) -> std::optional<MqttResponse> {
          return MqttResponse{type};
       });
}

std::optional<MqttBytes> sendAndReceiveResponse(ITransport &transport,
                                                mqtt::MqttBytes const &bytes) {

   bool const send_successful{transport.sendBytes(bytes)};

   if (!send_successful) {
      return std::nullopt;
   }

   return transport.receiveBytes();
}

} // namespace

bool Client::mqttConnect(ITransport &transport) {
   MqttBytes packet_bytes{packet::encodeConnect({.client_id = "client123"})};

   connected_ =
       sendAndReceiveResponse(transport, packet_bytes)
           .and_then(tryParseResponse)
           .and_then([](MqttResponse const resp) -> std::optional<bool> {
              return resp.type == MqttControlPackageType::kConnAck;
           })
           .value_or(false);

   return connected_;
}

bool Client::mqttPublish(ITransport &transport, PublishMessage const &publish_message) const {

   assert(publish_message.qos == QoS::kAtMostOnce &&
          "QoS > 0 is not supported yet.");
   assert(publish_message.retain == false && "Retain is not supported yet");

   MqttBytes packet_bytes{packet::encodePublish(publish_message)};

   return transport.sendBytes(packet_bytes);
}

bool Client::mqttSubscribe(ITransport &transport,
                           SubscribeTopics const &subscribe_topics) {
   MqttBytes packet_bytes{
       packet::encodeSubscribe(subscribe_topics, subscription_identifier_)};

   // TODO: Handle overflow here, a 0 identifier is not valid
   subscription_identifier_ += 1U;

   return sendAndReceiveResponse(transport, packet_bytes)
       .and_then(tryParseResponse)
       .and_then([](MqttResponse const resp) -> std::optional<bool> {
          return resp.type == MqttControlPackageType::kSubAck;
       })
       .value_or(false);
}

} // namespace mqtt
