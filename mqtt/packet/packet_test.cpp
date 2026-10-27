#include "mqtt/packet/packet.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <ranges>

namespace mqtt::packet::test {

class MqttConnectTest : public ::testing::Test {};

TEST_F(MqttConnectTest, EncodesMinimalConnect) {
   using ::testing::_;
   using ::testing::ElementsAre;

   EXPECT_THAT(packet::encodeConnect({.client_id = "Test"}),
               ElementsAre(0x10, _, 0x00, 0x04, 'M', 'Q', 'T', 'T', 0x04, _, _,
                           _, 0x00, 0x04, 'T', 'e', 's', 't'));
}

TEST_F(MqttConnectTest, EncodesKeepAliveBigEndian) {
   using ::testing::_;
   using ::testing::ElementsAre;

   auto const bytes = packet::encodeConnect({.keep_alive_seconds = 0x1234});

   EXPECT_GE(bytes.size(), 12U);
   EXPECT_THAT(std::span{bytes}.subspan(10, 2), ElementsAre(0x12, 0x34));
}

TEST_F(MqttConnectTest, EncodesZeroKeepAlive)
{
   using ::testing::_;
   using ::testing::ElementsAre;

   auto const bytes = packet::encodeConnect({.keep_alive_seconds = 0});

   EXPECT_GE(bytes.size(), 12U);
   EXPECT_THAT(std::span{bytes}.subspan(10, 2), ElementsAre(0x00, 0x00));

}

TEST_F(MqttConnectTest, EncodesCleanSessionInConnectFlags)
{
   using ::testing::_;
   using ::testing::ElementsAre;

   auto const bytes = packet::encodeConnect({.clean_session = true});

   EXPECT_GE(bytes.size(), 10U);
   EXPECT_EQ(bytes.at(9), 0x02);
}

TEST_F(MqttConnectTest, EncodesSessionContinuationInConnectFlags)
{
   using ::testing::_;
   using ::testing::ElementsAre;

   auto const bytes = packet::encodeConnect({.clean_session = false});

   EXPECT_GE(bytes.size(), 10U);
   EXPECT_EQ(bytes.at(9), 0x00);
}

} // namespace mqtt::packet::test
