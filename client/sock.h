#ifndef _MQTT_CLIENT_SOCK_H_
#define _MQTT_CLIENT_SOCK_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

#include <optional>
#include <vector>

#include "mqtt/transport.h"
#include "mqtt/types.h"

namespace sock {

using Socket = int;

namespace detail {

inline std::optional<Socket> connect(std::string const &address,
                                     int const port) {
   auto fd = ::socket(AF_INET, SOCK_STREAM, 0);

   if (fd < 0) {
      return std::nullopt;
   }

   sockaddr_in addr{};
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

   auto const conn_res =
   // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
       ::connect(fd, reinterpret_cast<sockaddr const *>(&addr), sizeof(addr));

   if (conn_res < 0) {
      return std::nullopt;
   }

   return fd;
}

inline bool send(Socket const fd, void const *const buf, size_t n) {
   auto const ret = ::send(fd, buf, n, 0U);
   return ret >= 0;
}

inline std::optional<std::vector<uint8_t>> tryReceive(Socket const fd) {
   constexpr size_t kBufSize{1000U};
   std::vector<uint8_t> buf(kBufSize);

   auto const ret = ::recv(fd, buf.data(), buf.size(), 0);

   if (ret > -1) {
      return buf;
   }
   return std::nullopt;
}

} // namespace detail

class SocketTransport : public mqtt::ITransport {
 public:
   bool connect(std::string const& address, int const port) {
      auto const fd = detail::connect(address, port);

      if (!fd.has_value()) {
         return false;
      }

      fd_ = fd.value();
      return true;
   }

   bool sendBytes(mqtt::MqttBytes const &bytes) override {
      return detail::send(fd_, bytes.data(), bytes.size());
   }

   std::optional<mqtt::MqttBytes> receiveBytes() override {
      return detail::tryReceive(fd_);
   }

 private:
   Socket fd_{};
};

} // namespace sock

#endif // _MQTT_CLIENT_SOCK_H_
