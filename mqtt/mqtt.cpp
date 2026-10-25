#include "mqtt/mqtt.h"
#include "mqtt/transport.h"
#include "mqtt/types.h"
#include <cassert>
#include <cstdint>
#include <limits>
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

class ByteWriter {
 public:
   void putU8(uint8_t const u8) { buf_.push_back(u8); }

   void putU16(uint16_t const u16) {
      buf_.push_back(static_cast<uint8_t>(u16 >> kOneByteInBits));
      buf_.push_back(static_cast<uint8_t>(u16));
   }

   void putVarByteInt(uint32_t value) {
      constexpr uint32_t kMaxSizeVariableByteInteger{0x0FFFFFFF};
      assert(value <= kMaxSizeVariableByteInteger &&
             "Value is larger than Variable Byte Integer limits");

      constexpr uint32_t kValuesPerByte{128U};
      constexpr uint32_t kContinuationBit{0x80};

      while (value >= kValuesPerByte) {
         uint8_t const encoded_byte{
             static_cast<uint8_t>((value % kValuesPerByte) | kContinuationBit)};
         buf_.push_back(encoded_byte);

         value /= kValuesPerByte;
      }
      buf_.push_back(static_cast<uint8_t>(value));
   }

   void putRawPayload(std::string_view const str) {
      buf_.insert(buf_.end(), str.begin(), str.end());
   }

   // TODO: Find a better name
   void putString(std::string_view const str) {
      assert(str.size() <= std::numeric_limits<uint16_t>().max());

      putU16(static_cast<uint16_t>(str.size()));
      putRawPayload(str);
   }

   void putBytes(std::span<uint8_t const> const bytes)
   {
      buf_.insert(buf_.end(), bytes.begin(), bytes.end());
   }

   std::vector<uint8_t> const &bytes() const { return buf_; }

 private:
   static constexpr uint8_t kOneByteInBits{8U};

   std::vector<uint8_t> buf_;
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

   ByteWriter body{};
   body.putString("MQTT");
   body.putU8(0x04);
   body.putU8(0x02);
   body.putU16(0x3C);
   body.putU16(0x00);

   ByteWriter packet{};
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

   ByteWriter body{};
   body.putString(publish_message.topic.value);
   // body.putU16(0x0); // TODO fix, only applicable for QoS 1 or 2
   // body.putU8(0x0);
   body.putRawPayload(publish_message.message);

   ByteWriter packet{};
   packet.putU8(static_cast<uint8_t>(MqttControlPackageType::kPublish));
   packet.putVarByteInt(body.bytes().size());
   packet.putBytes(body.bytes());

   return transport.sendBytes(packet.bytes());
}

} // namespace mqtt
