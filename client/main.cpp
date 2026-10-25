
#include <arpa/inet.h>
#include <array>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

#include "client/sock.h"
#include "mqtt/mqtt.h"
#include "mqtt/types.h"

namespace {

constexpr std::string kAddress{"127.0.0.1"};
constexpr int kPort{6666};

mqtt::PublishMessage createMessage(mqtt::Topic topic, std::string_view msg) {
   return mqtt::PublishMessage{.topic = topic, .message = msg};
}

} // namespace

int main(int argc, char **argv) {

   sock::SocketTransport socket{};
   bool const sock_connected{socket.connect(kAddress, kPort)};

   if (!sock_connected) {
      perror("Failed to connect to sock");
      return 1;
   }

   auto const connected{mqtt::mqttConnect(socket)};

   if (!connected) {
      perror("Mqtt connection failed");
      return 1;
   }

   std::cout << "Mqtt connected!\n";

   std::string line{};
   while (std::getline(std::cin, line)) {
      auto const sent{
          mqtt::mqttPublish(socket, createMessage({"test/topic"}, line))};

      if (!sent) {
         perror("Mqtt publish failed");
         return 1;
      }

      std::cout << "Mqtt published!\n";
   }

   auto const sent{
       mqtt::mqttPublish(socket, createMessage({"test/topic"}, "hello"))};

   if (!sent) {
      perror("Mqtt publish failed");
      return 1;
   }

   std::cout << "Mqtt published!\n";
}
