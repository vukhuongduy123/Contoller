#include "socketnode.h"
#include <array>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace {
constexpr WSADATA kDefaultWsadata{0};
constexpr int32_t kMaxBuffer     = 4096;
constexpr int32_t kThreadNoErr   = 0;
constexpr int32_t kLastErrOk     = 0;
constexpr int32_t kWsaStartupOk  = 0;
constexpr int32_t kReqWinsockVer = 2;
const char*       kLocalAddress  = "127.0.0.1";
constexpr int32_t kDefaultPort   = 4444;
constexpr int32_t kServerPort    = 8080;
constexpr int32_t kStatusPort    = 8888;
constexpr int32_t kTimeReval     = 1000;
DWORD             timeout[2]     = {static_cast<DWORD>(1000), 0};
const int         size           = sizeof(DWORD);
constexpr char    kBroadCastMode = 1;
const std::string kClose         = "close";
constexpr int32_t kNoErr         = 0;
const std::string kNewConnection = "NewConnection";
const std::string kServer        = "server";
const std::string kClient        = "client";
const std::string kServerAccpted = "ServerAcceted";

}  // namespace
DWORD WINAPI ReadingThreadUDP(LPVOID param);

std::string GetTime() {
    struct tm newtime;
    auto      t = std::time(nullptr);
    localtime_s(&newtime, &t);

    std::ostringstream oss;
    oss << "Current time: " << std::put_time(&newtime, "%d-%m-%Y %H-%M");
    auto str = oss.str();

    return str;
}

bool SocketConnectToControl(SocketNode& s, const std::string& text) {
    shutdown(s.socket_, SD_BOTH);
    s.CreateSocket(SOCK_DGRAM, 0);
    s.set_socketaddr(AF_INET, kDefaultPort, kLocalAddress);
    s.SetOpt(SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(timeout), size);
    s.SendTo(text);
    std::string res;
    s.RecvFromControl(res);
    return true;
}

bool SocketServer(SocketNode& s) {
    shutdown(s.socket_, SD_BOTH);
    s.CreateSocket(SOCK_DGRAM, 0);
    s.set_socketaddr(AF_INET, kServerPort, kLocalAddress);
    s.SetOpt(SOL_SOCKET, SO_BROADCAST, &kBroadCastMode, sizeof(kBroadCastMode));
    if (s.Bind() == INVALID_SOCKET) {
        return false;
    }
    s.set_socketaddr(AF_INET, kServerPort, INADDR_BROADCAST);
    return true;
}

bool SocketClient(SocketNode& s, const std::string& ip) {
    shutdown(s.socket_, SD_BOTH);
    s.CreateSocket(SOCK_DGRAM, 0);
    s.set_socketaddr(AF_INET, kServerPort, ip.c_str());
    s.SetOpt(SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(timeout), size);
    s.SetOpt(SOL_SOCKET, SO_BROADCAST, &kBroadCastMode, sizeof(kBroadCastMode));
    if (s.Bind() == INVALID_SOCKET) {
        return false;
    }
    return true;
}

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
    SocketConnectToControl(*this, kNewConnection);
    int32_t curr_role = get_role();
    while (curr_role != unknown_role_) {
        if (curr_role == server_) {
            if (!SocketServer(*this))
                break;
            std::cout << "Server is running";
            while (true) {
                std::string text = GetTime();
                SendTo(text.c_str());
            }
        }
        if (curr_role == client_) {
            int32_t     res = 0;
            std::string text(kMaxBuffer, '\0');
            std::string ip;
            std::cout << "Enter local ip: ";
            std::cin >> ip;
            if (!SocketClient(*this, ip))
                break;
            do {
                res = RecvFrom(text);
                if (res == INVALID_SOCKET &&
                    WSAGetLastError() == WSAETIMEDOUT) {
                    SocketConnectToControl(*this, kNewConnection);
                    curr_role = get_role();
                } else {
                    std::cout << text.data() << std::endl;
                }
            } while (res != INVALID_SOCKET);
        }
    }
}

SocketNode::~SocketNode() {
    shutdown(socket_, SD_BOTH);
    closesocket(socket_);
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
                                const char*   sin_addr) noexcept {
    socket_addr_.sin_family           = sin_family;
    socket_addr_.sin_port             = htons(sin_port);
    socket_addr_.sin_addr.S_un.S_addr = inet_addr(sin_addr);
}
void SocketNode::set_socketaddr(const int16_t  sin_family,
                                const int16_t  sin_port,
                                const uint32_t sin_addr) noexcept {
    socket_addr_.sin_family           = sin_family;
    socket_addr_.sin_port             = htons(sin_port);
    socket_addr_.sin_addr.S_un.S_addr = sin_addr;
}

void SocketNode::set_socketaddr(const sockaddr_in& socket_addr) noexcept {
    socket_addr_ = socket_addr;
}

int32_t SocketNode::Bind() noexcept {
    int32_t result = bind(
        socket_, reinterpret_cast<sockaddr*>(&socket_addr_), sizeof(sockaddr));
    return result;
}

int32_t SocketNode::RecvFromControl(std::string& text) noexcept {
    std::string buf(kMaxBuffer, '\0');
    int         size   = sizeof(socket_cli_addr_);
    int32_t     result = recvfrom(socket_,
                              buf.data(),
                              kMaxBuffer,
                              0,
                              reinterpret_cast<sockaddr*>(&socket_cli_addr_),
                              &size);
    if (result == INVALID_SOCKET)
        return result;
    text = buf.data();
    if (std::strcmp(buf.c_str(), kServer.c_str()) == 0) {
        role_ = server_;

        SendTo(kServerAccpted);

        SOCKET      tmp_socket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in tmp{};
        tmp.sin_family           = AF_INET;
        tmp.sin_port             = htons(kStatusPort);
        tmp.sin_addr.S_un.S_addr = inet_addr(kLocalAddress);
        bind(tmp_socket, reinterpret_cast<sockaddr*>(&tmp), sizeof(tmp));
        listen(tmp_socket, SOMAXCONN);
        status_socket_ = accept(tmp_socket, nullptr, nullptr);
        closesocket(tmp_socket);

    } else if (std::strcmp(buf.c_str(), kClient.c_str()) == 0) {
        role_ = client_;
    }

    return result;
}

int32_t SocketNode::ConnectToControl() noexcept {
    if (connect(socket_,
                reinterpret_cast<sockaddr*>(&socket_addr_),
                sizeof(socket_addr_)) == SOCKET_ERROR)
        return WSAGetLastError();
    return 0;
}

int32_t SocketNode::SendTo(const std::string& text) noexcept {
    int32_t result = sendto(socket_,
                            text.c_str(),
                            text.size(),
                            0,
                            reinterpret_cast<SOCKADDR*>(&socket_addr_),
                            sizeof(socket_addr_));
    return result;
}
int32_t SocketNode::RecvFrom(std::string& text) noexcept {
    int     size   = sizeof(socket_addr_);
    int32_t result = recvfrom(socket_,
                              text.data(),
                              kMaxBuffer,
                              0,
                              reinterpret_cast<sockaddr*>(&socket_addr_),
                              &size);
    return result;
}

int32_t SocketNode::get_role() noexcept {
    return role_;
}
