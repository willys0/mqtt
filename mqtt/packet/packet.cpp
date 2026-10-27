#include "mqtt/packet/packet.h"

#include "mqtt/codec/byte_writer.h"

namespace mqtt::packet {

namespace {

// MQTT 3.1.1
constexpr uint8_t kMqttProtocolLevelByte{0x04};

enum class MqttControlPackageType : uint8_t {
   kInvalid = 0,
   kConnect = 1 << 4,
   kConnAck = 2 << 4,
   kPublish = 3 << 4,
   kPubAck = 4 << 4,
};

uint8_t makeConnectFlags(bool const clean_session) {
   uint8_t out_byte{};
   if (clean_session) {
      out_byte |= 0x02;
   }

   return out_byte;
}

} // namespace

MqttBytes encodeConnect(ConnectOptions const &connect_options) {
   codec::ByteWriter body{};
   body.putString("MQTT");
   body.putU8(kMqttProtocolLevelByte);
   body.putU8(makeConnectFlags(connect_options.clean_session));
   body.putU16(connect_options.keep_alive_seconds);
   body.putString(connect_options.client_id);

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
   body.putBytes(publish_message.message);

   codec::ByteWriter packet{};
   packet.putU8(static_cast<uint8_t>(MqttControlPackageType::kPublish));
   packet.putVarByteInt(body.bytes().size());
   packet.putBytes(body.bytes());

   return packet.bytes();
}

} // namespace mqtt::packet
