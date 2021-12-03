#include "controller.h"

#include <String>
#include <array>
namespace {
constexpr char kBroadCastMode = 1;
constexpr int32_t kMaxBuffer = 4096;
constexpr int32_t kThreadNoErr = 0;
constexpr int32_t kLastErrOk = 0;
constexpr WSADATA kDefaultWsadata{0};
constexpr int32_t kWsaStartupOk = 0;
constexpr int32_t kReqWinsockVer = 2;
constexpr int32_t kFirstConnection = 1;
constexpr int32_t kTimeReval = 100;
constexpr int32_t kServerPort = 8080;
constexpr int32_t kStatusPort = 8888;
DWORD timeout[2] = {static_cast<DWORD>(1000), 0};
const int size = sizeof(DWORD);
const char* kLocalAddress = "127.0.0.1";
std::vector<std::string> default_command{"Close", "Add", "ChangeToServer",
                                         "SetNewRole"};
const std::string kNewAddressCommand = "NewConnection";
const std::string kServerAcctepted = "ServerAccepted";
const std::string kServerNotRespon = "ServerIsNotRespon";
std::vector<std::string> set_role = {"unknown_role", "client", "server"};
}  // namespace

DWORD WINAPI ReadingThreadTCP(LPVOID param) {
  Controller* socket_node = static_cast<Controller*>(param);
  int32_t size = sizeof(socket_node->controller_socket_.socket_cli_addr_);
  int32_t result{};
  std::string buf(kMaxBuffer, '\0');
  do {
    result = recvfrom(socket_node->controller_socket_.socket_, buf.data(),
                      buf.size(), 0,
                      reinterpret_cast<sockaddr*>(
                          &socket_node->controller_socket_.socket_cli_addr_),
                      &size);
    buf[result] = '\0';

    if (std::strcmp(buf.c_str(), kNewAddressCommand.c_str()) == 0) {
      if (send(socket_node->controller_socket_.status_socket_, buf.data(),
               buf.size(), 0) == SOCKET_ERROR) {
        sendto(socket_node->controller_socket_.socket_,
               set_role[server_].data(), set_role[server_].size(), 0,
               reinterpret_cast<sockaddr*>(
                   &socket_node->controller_socket_.socket_cli_addr_),
               size);
        recvfrom(socket_node->controller_socket_.socket_, buf.data(),
                 buf.size(), 0,
                 reinterpret_cast<sockaddr*>(
                     &socket_node->controller_socket_.socket_cli_addr_),
                 &size);
        shutdown(socket_node->controller_socket_.status_socket_, SD_BOTH);
        socket_node->controller_socket_.status_socket_ =
            socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in tmp;
        tmp.sin_family = AF_INET;
        tmp.sin_port = htons(kStatusPort);
        tmp.sin_addr.S_un.S_addr = inet_addr(kLocalAddress);

        int check = connect(socket_node->controller_socket_.status_socket_,
                            reinterpret_cast<sockaddr*>(&tmp), sizeof(tmp));

      } else {
        sendto(socket_node->controller_socket_.socket_,
               set_role[client_].data(), set_role[client_].size(), 0,
               reinterpret_cast<sockaddr*>(
                   &socket_node->controller_socket_.socket_cli_addr_),
               size);
      }
    }
  } while (true);
  return kThreadNoErr;
}

bool Controller::Init() noexcept {
  if (WSAStartup(MAKEWORD(kReqWinsockVer, kReqWinsockVer), &wsadata_) ==
          kWsaStartupOk &&
      LOBYTE(wsadata_.wVersion) >= kReqWinsockVer)
    return true;
  return false;
}

Controller::Controller() noexcept
    : controller_socket_(), wsadata_(kDefaultWsadata) {
  Init();
  controller_socket_.CreateSocket(SOCK_DGRAM, 0);
  controller_socket_.set_socketaddr(AF_INET, kDefaultPort, ADDR_ANY);
  bind(controller_socket_.socket_,
       reinterpret_cast<sockaddr*>(&controller_socket_.socket_addr_),
       sizeof(sockaddr_in));
  HANDLE thread = CreateThread(nullptr, 0, &ReadingThreadTCP, this, 0, 0);
  WaitForSingleObject(thread, INFINITE);
  CloseHandle(thread);
}