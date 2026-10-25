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

   void putRawPayload(std::string_view const str) {
      buf_.insert(buf_.end(), str.begin(), str.end());
   }

   // TODO: Find a better name
   void putString(std::string_view const str) {
      assert(str.size() <= std::numeric_limits<uint16_t>().max());

      putU16(static_cast<uint16_t>(str.size()));
      putRawPayload(str);
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

   std::vector<uint8_t> body_bytes{body.bytes()};

   std::vector<uint8_t> bytes{};
   bytes.push_back(static_cast<uint8_t>(MqttControlPackageType::kConnect));
   bytes.push_back(body_bytes.size()); // TODO: This can be more then 1 byte
   bytes.insert(bytes.end(), body_bytes.begin(), body_bytes.end());

   return sendAndReceiveResponse(transport, bytes)
       .and_then(tryParseResponse)
       .and_then([](MqttResponse const resp) -> std::optional<bool> {
          return resp.type == MqttControlPackageType::kConnAck;
       })
       .value_or(false);
}

bool mqttPublish(ITransport &transport, Topic topic,
                 std::string_view msg) {

   // Var header: Topic Name, Packet Identifier, and Properties
   ByteWriter body{};
   body.putString(topic.value);
   // body.putU16(0x0); // TODO fix, only applicable for QoS 1 or 2
   // body.putU8(0x0);
   body.putRawPayload(msg);

   // Payload
   // The Payload contains the Application Message that is being published. The
   // content and format of the data is application specific. The length of the
   // Payload can be calculated by subtracting the length of the Variable Header
   // from the Remaining Length field that is in the Fixed Header. It is valid
   // for a PUBLISH packet to contain a zero length Payload.
   // TODO: add here, let's just have it be 0 for now

   std::vector<uint8_t> bytes{};
   bytes.push_back(static_cast<uint8_t>(MqttControlPackageType::kPublish));
   bytes.push_back(body.bytes().size()); // TODO: This can be more then 1 byte
   bytes.insert(bytes.end(), body.bytes().begin(), body.bytes().end());
   // TODO: handle QoS, Dup flag, Retain. For now just go for QoS 0 always

   return transport.sendBytes(bytes);
}

} // namespace mqtt
