#include "controller.h"
#include <time.h>
#include <iomanip>
#include <sstream>
#include "socket.h"
namespace {
constexpr char    kBroadCastMode = 1;
DWORD             timeout[2]     = {static_cast<DWORD>(1000), 0};
constexpr int32_t kTimeoutSize   = sizeof(DWORD);
constexpr int32_t kMaxBuffer     = 4096;

}  // namespace

std::string GetTime() {
    struct tm newtime;
    auto      t = std::time(nullptr);
    localtime_s(&newtime, &t);

    std::ostringstream oss;
    oss << "Current time: " << std::put_time(&newtime, "%d-%m-%Y %H-%M");

    return oss.str();
}

int32_t ConverToRole(const std ::string& role) {
    if (role == "server")
        return kServer;
    else if (role == "client")
        return kClient;
    else
        return kUnknownRole;
}

Controller::Controller(const std::string& role) noexcept
    : socket_node_(), logger() {
    int32_t curr_role = ConverToRole(role);
    if (curr_role == kUnknownRole)
        return;
    socket_node_.set_role(curr_role);
}

int32_t Controller::Run(const std::string& ip) noexcept {
    int32_t     curr_role = socket_node_.get_role();
    int32_t     res       = 0;
    std::string text(kMaxBuffer, '\0');
    while (curr_role != kUnknownRole) {
        if (curr_role == kServer) {
            logger.Write("server is running\n");
            socket_node_.ShutDown();
            socket_node_.set_socket(AF_INET, SOCK_DGRAM, 0);
            socket_node_.set_socket_addr(
                AF_INET, socket_node_.kDefaultPort, socket_node_.kLocalAddress);
            socket_node_.SetOpt(SOL_SOCKET,
                                SO_BROADCAST,
                                &kBroadCastMode,
                                sizeof(kBroadCastMode));
            if (socket_node_.Bind() == SOCKET_ERROR)
                return WSAGetLastError();

            socket_node_.set_socket_addr(
                AF_INET, socket_node_.kDefaultPort, INADDR_BROADCAST);
            do {
                std::string timetext = GetTime();
                socket_node_.SendTo(timetext);
            } while (true);
        }
        if (curr_role == kClient) {
            socket_node_.ShutDown();
            socket_node_.set_socket(AF_INET, SOCK_DGRAM, 0);
            socket_node_.set_socket_addr(
                AF_INET, socket_node_.kDefaultPort, ip.c_str());
            socket_node_.SetOpt(SOL_SOCKET,
                                SO_RCVTIMEO,
                                reinterpret_cast<char*>(&timeout),
                                kTimeoutSize);
            socket_node_.SetOpt(SOL_SOCKET,
                                SO_BROADCAST,
                                &kBroadCastMode,
                                sizeof(kBroadCastMode));
            if (socket_node_.Bind() == SOCKET_ERROR)
                return WSAGetLastError();

            do {
                res = socket_node_.RecvFrom(text);
                if (res == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
                    socket_node_.set_role(kServer);
                else if (res == SOCKET_ERROR)
                    return WSAGetLastError();
                else {
                    logger.Write(text.data());
                    logger.Write("\n");
                }
            } while (res != SOCKET_ERROR);
        }
        curr_role = socket_node_.get_role();
    }
    return 0;
}