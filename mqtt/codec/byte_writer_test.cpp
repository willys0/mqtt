#include "mqtt/codec/byte_writer.h"

#include <gtest/gtest.h>

namespace mqtt::codec::test {

TEST(ByteWriterTest, NoWritesGivesEmptyBytes) {
   ByteWriter writer{};

   EXPECT_EQ(writer.bytes().size(), 0U);
}

TEST(ByteWriterTest, WriteU8AddsOneByte) {
   ByteWriter writer{};

   constexpr uint8_t kByteToWrite{123U};
   writer.putU8(kByteToWrite);

   EXPECT_EQ(writer.bytes().size(), 1U);
   EXPECT_EQ(writer.bytes().at(0), kByteToWrite);
}

TEST(ByteWriterTest, WriteU16AddsTwoBytes) {
   ByteWriter writer{};

   constexpr uint16_t kValueToWrite{512U};
   writer.putU16(kValueToWrite);

   EXPECT_EQ(writer.bytes().size(), 2U);
   EXPECT_EQ(writer.bytes().at(0), kValueToWrite >> 8U);
   EXPECT_EQ(writer.bytes().at(1), kValueToWrite & 0xFF);
}

TEST(ByteWriterTest, WriteBytesAddsBytes) {
   ByteWriter writer{};

   std::vector<uint8_t> bytes_to_write{{1, 2, 3, 4}};
   writer.putBytes(bytes_to_write);

   EXPECT_EQ(writer.bytes().size(), 4U);
   EXPECT_EQ(writer.bytes(), bytes_to_write);
}

TEST(ByteWriterTest, WriteStringAddsLengthAndBytes) {
   ByteWriter writer{};

   std::string string_to_write{"Test string"};
   writer.putString(string_to_write);

   constexpr size_t kU16Size{2U};

   EXPECT_EQ(writer.bytes().size(), kU16Size + string_to_write.size());

   uint16_t const msb{writer.bytes().at(0)};
   uint16_t const lsb{writer.bytes().at(1)};

   uint16_t const size_bytes{static_cast<uint16_t>((msb << 8U) | (lsb))};

   EXPECT_EQ(size_bytes, string_to_write.size());
}

struct VarByteIntTestCase {
   uint32_t value;
   std::vector<uint8_t> expected;
};

class VarByteIntTest : public ::testing::TestWithParam<VarByteIntTestCase> {};

TEST_P(VarByteIntTest, WriteVarByteAddsExpectedBytes) {
   auto const &[value, expected] = GetParam();

   ByteWriter writer{};
   writer.putVarByteInt(value);

   EXPECT_EQ(writer.bytes(), expected);
}

INSTANTIATE_TEST_SUITE_P(
    VarByteIntBoundaries, VarByteIntTest,
    ::testing::Values(VarByteIntTestCase{0U, {0x00}},
                      VarByteIntTestCase{127U, {0x7F}},
                      VarByteIntTestCase{128U, {0x80, 0x01}},
                      VarByteIntTestCase{16'383U, {0xFF, 0x7F}},
                      VarByteIntTestCase{16'384U, {0x80, 0x80, 0x01}},
                      VarByteIntTestCase{2'097'151U, {0xFF, 0xFF, 0x7F}},
                      VarByteIntTestCase{2'097'152U, {0x80, 0x80, 0x80, 0x01}},
                      VarByteIntTestCase{268'435'455U,
                                         {0xFF, 0xFF, 0xFF, 0x7F}}));

} // namespace mqtt::codec::test
