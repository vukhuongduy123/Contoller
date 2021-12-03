#ifndef REFACTOR_SOCKETNODE_H
#define REFACTOR_SOCKETNODE_H
#include <winsock2.h>
#include <iostream>
#include <cstdint>
enum Role { unknown_role_, client_, server_ };
class Controller;
class SocketNode {
 protected:
  static constexpr int32_t kDefaultSocketAdrr = 0;
  sockaddr_in socket_addr_,socket_cli_addr_;
  int32_t role_;
  WSADATA wsadata_;
  HANDLE thread_;

  bool Init() noexcept;

 public:
  SOCKET socket_, status_socket_;

  SocketNode() noexcept;
  SocketNode(SOCKET, sockaddr_in, const int32_t) noexcept;
  SocketNode(const SocketNode&) noexcept;
  SocketNode& operator=(const SocketNode&) = default;
  SocketNode& operator=(SocketNode&&) = default;

  bool CreateSocket(const int32_t, const int32_t) noexcept;
  void set_socketaddr(const int16_t, const int16_t, const char*) noexcept;
  void set_socketaddr(const int16_t, const int16_t, const uint32_t) noexcept;
  void set_socketaddr(const sockaddr_in&) noexcept;

  int32_t Bind() noexcept;
  int32_t SetOpt(int32_t,int32_t,const char*,int32_t) noexcept;
  int32_t RecvFromControl(std::string&) noexcept;
  int32_t ConnectToControl() noexcept;
  int32_t get_role() noexcept;
  int32_t SendTo(const std::string& ) noexcept;
  int32_t RecvFrom(std::string& ) noexcept;

  friend class Controller;
  friend DWORD WINAPI ReadingThreadTCP(LPVOID);
 public:
  ~SocketNode() ;
};

#endif  // REFACTOR_SOCKETNODE_H
