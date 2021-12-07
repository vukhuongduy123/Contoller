#ifndef CONTROLLER_CONTROLLER_SOCKET_H
#define CONTROLLER_CONTROLLER_SOCKET_H
#include <winsock2.h>
#include <cstdint>
#include <iostream>
enum role { kUnknownRole, kServer, kClient };
class Socket {
    SOCKET  socket_;
    WSAData wsadata_;
    int32_t role_;

   public:
    sockaddr_in socket_addr_, socket_cli_;

    Socket() noexcept;
    ~Socket();
    Socket(const Socket&) = delete;
    Socket(Socket&&)      = delete;
    Socket& operator=(const Socket) = delete;
    Socket& operator=(Socket&&) = delete;

    const int32_t kDefaultPort  = 4444;
    const int32_t kClientPort   = 8080;
    const char*   kLocalAddress = "127.0.0.1";

    int32_t set_socket(int32_t af, int32_t type, int32_t protocol) noexcept;
    int32_t get_socket() noexcept;
    void    set_socket_addr(const int16_t sin_family, const int16_t sin_port,
                            const char* sin_addr) noexcept;
    void    set_socket_addr(const int16_t sin_family, const int16_t sin_port,
                            const uint32_t sin_addr) noexcept;
    int32_t SetOpt(int32_t level, int32_t opt, const char* optval,
                   int32_t optlen) noexcept;
    int32_t Bind() noexcept;
    int32_t SendTo(const std::string& text) noexcept;
    int32_t RecvFrom(std::string& text) noexcept;
    void    set_role(const int32_t role) noexcept;
    int32_t get_role() noexcept;
    bool    Inti() noexcept;
    bool    Release() noexcept;
    bool    ShutDown() noexcept;
};
#endif  // !CONTROLLER_CONTROLLER_SOCKET_H
