#include "mqtt/packet/packet.h"

#include "mqtt/codec/byte_writer.h"

namespace mqtt::packet {

namespace {

enum class MqttControlPackageType : uint8_t {
   kInvalid = 0,
   kConnect = 1 << 4,
   kConnAck = 2 << 4,
   kPublish = 3 << 4,
   kPubAck = 4 << 4,
};

} // namespace

MqttBytes encodeConnect() {
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

   return packet.bytes();
}

MqttBytes encodePublish(PublishMessage const &publish_message) {
   codec::ByteWriter body{};
   body.putString(publish_message.topic.value);
   // body.putU16(0x0); // TODO fix, only applicable for QoS 1 or 2
   // body.putU8(0x0);
   body.putRawPayload(publish_message.message);

   codec::ByteWriter packet{};
   packet.putU8(static_cast<uint8_t>(MqttControlPackageType::kPublish));
   packet.putVarByteInt(body.bytes().size());
   packet.putBytes(body.bytes());

   return packet.bytes();
}

} // namespace mqtt::packet
