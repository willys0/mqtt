#include "mqtt/mqtt.h"
#include "mqtt/transport.h"
#include "mqtt/types.h"
#include "mqtt/codec/byte_writer.h"
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
};

struct MqttResponse {
   MqttControlPackageType type{};
};

bool isValidResponse(MqttControlPackageType const type) {
   switch (type) {
   case MqttControlPackageType::kConnAck:
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

bool mqttConnect(ITransport &transport) {

   codec::ByteWriter body{};
   body.putString("MQTT");
   body.putU8(0x04);
   body.putU8(0x02);
   body.putU16(0x3C);
   body.putU16(0x00);

   codec::ByteWriter packet{};
   packet.putU8(static_cast<uint8_t>(MqttControlPackageType::kConnect));
   packet.putVarByteInt(body.bytes().size());
   packet.putBytes(body.bytes());

   return sendAndReceiveResponse(transport, packet.bytes())
       .and_then(tryParseResponse)
       .and_then([](MqttResponse const resp) -> std::optional<bool> {
          return resp.type == MqttControlPackageType::kConnAck;
       })
       .value_or(false);
}

bool mqttPublish(ITransport &transport, PublishMessage const &publish_message) {

   assert(publish_message.qos == QoS::kAtMostOnce &&
          "QoS > 0 is not supported yet.");
   assert(publish_message.retain == false && "Retain is not supported yet");

   codec::ByteWriter body{};
   body.putString(publish_message.topic.value);
   // body.putU16(0x0); // TODO fix, only applicable for QoS 1 or 2
   // body.putU8(0x0);
   body.putRawPayload(publish_message.message);

   codec::ByteWriter packet{};
   packet.putU8(static_cast<uint8_t>(MqttControlPackageType::kPublish));
   packet.putVarByteInt(body.bytes().size());
   packet.putBytes(body.bytes());

   return transport.sendBytes(packet.bytes());
}

} // namespace mqtt
