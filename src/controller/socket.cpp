#include "socket.h"

namespace {
constexpr int32_t kWsaStartupOk  = 0;
constexpr int32_t kWsaCleanOk    = 0;
constexpr int32_t kReqWinsockVer = 2;
constexpr int32_t kMaxBuffer     = 4096;
}  // namespace

Socket::Socket() noexcept
    : socket_(INVALID_SOCKET),
      socket_cli_(),
      wsadata_(),
      socket_addr_(),
      role_(kUnknownRole) {
    Inti();
}

Socket::~Socket() {
    Release();
    shutdown(socket_, SD_BOTH);
    closesocket(socket_);
    Release();
}

bool Socket::Inti() noexcept {
    if (WSAStartup(MAKEWORD(kReqWinsockVer, kReqWinsockVer), &wsadata_) ==
            kWsaStartupOk &&
        LOBYTE(wsadata_.wVersion) >= kReqWinsockVer)
        return true;
    return false;
}

bool Socket::Release() noexcept {
    if (WSACleanup() == kWsaCleanOk)
        return true;
    return false;
}
int32_t Socket::get_socket() noexcept {
    return socket_;
}

int32_t Socket::set_socket(int32_t af, int32_t type,
                           int32_t protocol) noexcept {
    socket_ = socket(af, type, protocol);
    return socket_;
}

void Socket::set_socket_addr(const int16_t sin_family, const int16_t sin_port,
                             const char* sin_addr) noexcept {
    socket_addr_.sin_family           = sin_family;
    socket_addr_.sin_port             = htons(sin_port);
    socket_addr_.sin_addr.S_un.S_addr = inet_addr(sin_addr);
}

void Socket::set_socket_addr(const int16_t sin_family, const int16_t sin_port,
                             const uint32_t sin_addr) noexcept {
    socket_addr_.sin_family           = sin_family;
    socket_addr_.sin_port             = htons(sin_port);
    socket_addr_.sin_addr.S_un.S_addr = sin_addr;
}

int32_t Socket::SetOpt(int32_t level, int32_t opt, const char* optval,
                       int32_t optlen) noexcept {
    int32_t result = setsockopt(socket_, level, opt, optval, optlen);
    return result;
}

int32_t Socket::Bind() noexcept {
    int res = bind(
        socket_, reinterpret_cast<sockaddr*>(&socket_addr_), sizeof(sockaddr));
    return res;
}

int32_t Socket::SendTo(const std::string& text) noexcept {
    int32_t result = sendto(socket_,
                            text.c_str(),
                            text.size(),
                            0,
                            reinterpret_cast<SOCKADDR*>(&socket_addr_),
                            sizeof(socket_addr_));
    return result;
}

int32_t Socket::RecvFrom(std::string& text) noexcept {
    int     sizesock_addr = sizeof(socket_addr_);
    int32_t result        = recvfrom(socket_,
                              text.data(),
                              kMaxBuffer,
                              0,
                              reinterpret_cast<sockaddr*>(&socket_cli_),
                              &sizesock_addr);
    return result;
}

int32_t Socket::get_role() noexcept {
    return role_;
}

void Socket::set_role(const int32_t role) noexcept {
    role_ = role;
}
bool Socket::ShutDown() noexcept {
    shutdown(socket_, SD_BOTH);
    return true;
}