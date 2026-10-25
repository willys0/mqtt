#ifndef MQTT_CODEC_H_
#define MQTT_CODEC_H_

#include <cassert>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

namespace mqtt::codec {

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

   void putString(std::string_view const str) {
      assert(str.size() <= std::numeric_limits<uint16_t>().max());

      putU16(static_cast<uint16_t>(str.size()));
      putRawPayload(str);
   }

   void putBytes(std::span<uint8_t const> const bytes) {
      buf_.insert(buf_.end(), bytes.begin(), bytes.end());
   }

   [[nodiscard]]
   std::vector<uint8_t> const &bytes() const {
      return buf_;
   }

 private:
   static constexpr uint8_t kOneByteInBits{8U};

   std::vector<uint8_t> buf_;
};

} // namespace mqtt::codec

#endif // MQTT_CODEC_H_
