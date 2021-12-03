#include "socketnode.h"

#include <array>
#include <cctype>
#include <iostream>
#include <string>
namespace {
constexpr int32_t kMaxBuffer = 4096;
constexpr int32_t kThreadNoErr = 0;
constexpr int32_t kLastErrOk = 0;
constexpr WSADATA kDefaultWsadata{0};
constexpr int32_t kWsaStartupOk = 0;
constexpr int32_t kReqWinsockVer = 2;
const char* kLocalAddress = "127.0.0.1";
constexpr int32_t kServerPort = 8080;
constexpr int32_t kStatusPort = 8888;

constexpr int32_t kTimeReval = 1000;
DWORD timeout[2] = {static_cast<DWORD>(1000), 0};
const int size = sizeof(DWORD);
constexpr char kBroadCastMode = 1;
const std::string kClose = "close";
constexpr int32_t kNoErr = 0;


}  // namespace
DWORD WINAPI ReadingThreadTCP(LPVOID param);

bool SocketNode::Init() noexcept {
  if (WSAStartup(MAKEWORD(kReqWinsockVer, kReqWinsockVer), &wsadata_) ==
          kWsaStartupOk &&
      LOBYTE(wsadata_.wVersion) >= kReqWinsockVer)
    return true;

  return false;
}
SocketNode::SocketNode() noexcept
    : socket_(INVALID_SOCKET),
      status_socket_(INVALID_SOCKET),
      socket_addr_(),
      socket_cli_addr_(),
      role_(unknown_role_),
      wsadata_(kDefaultWsadata),
      thread_(nullptr) {
  Init();
}

SocketNode::SocketNode(SOCKET socket, sockaddr_in socket_addr,
                       const int32_t role) noexcept
    : socket_(socket),
      status_socket_(INVALID_SOCKET),
      socket_addr_(socket_addr),
      socket_cli_addr_(),
      role_(role),
      wsadata_(kDefaultWsadata),
      thread_(nullptr) {
  Init();
}

SocketNode::~SocketNode() {
  shutdown(socket_,SD_BOTH);
  closesocket(socket_);
}

SocketNode::SocketNode(const SocketNode& socketnode) noexcept
    : socket_(socketnode.socket_),
      status_socket_(socketnode.status_socket_),
      socket_addr_(socketnode.socket_addr_),
      socket_cli_addr_(),
      role_(socketnode.role_),
      wsadata_(kDefaultWsadata),
      thread_(nullptr) {}

bool SocketNode::CreateSocket(const int32_t type,
                              const int32_t protocal) noexcept {
  if ((socket_ = socket(AF_INET, type, protocal)) == INVALID_SOCKET)
    return false;

  return true;
}

int32_t SocketNode::SetOpt(int32_t level, int32_t opt, const char* optval,
                           int32_t optlen) noexcept {
  int32_t result = setsockopt(socket_, level, opt, optval, optlen);
  return result;
}

void SocketNode::set_socketaddr(const int16_t sin_family,
                                const int16_t sin_port,
                                const char* sin_addr) noexcept {
  socket_addr_.sin_family = sin_family;
  socket_addr_.sin_port = htons(sin_port);
  socket_addr_.sin_addr.S_un.S_addr = inet_addr(sin_addr);
}
void SocketNode::set_socketaddr(const int16_t sin_family,
                                const int16_t sin_port,
                                const uint32_t sin_addr) noexcept {
  socket_addr_.sin_family = sin_family;
  socket_addr_.sin_port = htons(sin_port);
  socket_addr_.sin_addr.S_un.S_addr = sin_addr;
}

void SocketNode::set_socketaddr(const sockaddr_in& socket_addr) noexcept {
  socket_addr_ = socket_addr;
}

int32_t SocketNode::Bind() noexcept {
  int32_t result = bind(socket_, reinterpret_cast<sockaddr*>(&socket_addr_),
                        sizeof(sockaddr));
  return result;
}

int32_t SocketNode::RecvFromControl(std::string& text) noexcept {
  std::string buf(kMaxBuffer,'\0');
  int size = sizeof(socket_cli_addr_);
  int32_t result = recvfrom(socket_, buf.data(), kMaxBuffer, 0,
               reinterpret_cast<sockaddr*>(&socket_cli_addr_), &size);
  if (result == INVALID_SOCKET) return result;
  text = buf.data();
  if (std::strcmp(buf.c_str(), "server") == 0) {
    role_ = server_;

    SendTo("ServerAcceted");

    SOCKET tmp_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in tmp;
    tmp.sin_family = AF_INET;
    tmp.sin_port = htons(kStatusPort);
    tmp.sin_addr.S_un.S_addr = inet_addr(kLocalAddress);
    bind(tmp_socket, reinterpret_cast<sockaddr*>(&tmp), sizeof(tmp));
    listen(tmp_socket, SOMAXCONN);
    status_socket_ = accept(tmp_socket, nullptr, nullptr);
    closesocket(tmp_socket);


  }
  else if (std::strcmp(buf.c_str(), "client") == 0) {
    role_ = client_;
  }

  return result;
}

int32_t SocketNode::ConnectToControl() noexcept {
  if (connect(socket_, reinterpret_cast<sockaddr*>(&socket_addr_),
              sizeof(socket_addr_)) == SOCKET_ERROR)
    return WSAGetLastError();
  return 0;
}

int32_t SocketNode::SendTo(const std::string& text) noexcept {
  int32_t result =
      sendto(socket_, text.c_str(), text.size() , 0,
             reinterpret_cast<SOCKADDR*>(&socket_addr_), sizeof(socket_addr_));
  return result;
}
int32_t SocketNode::RecvFrom(std::string& text) noexcept {
  int size = sizeof(socket_addr_);
  int32_t result = recvfrom(socket_, text.data(), kMaxBuffer, 0,
               reinterpret_cast<sockaddr*>(&socket_addr_), &size);
  return result;
}

int32_t SocketNode::get_role() noexcept { return role_; }

