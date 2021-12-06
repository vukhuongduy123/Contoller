#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "controller.h"
#include "socketnode.h"
#include "sstream"

namespace {
const char*       kLocalAddress  = "127.0.0.1";
constexpr int32_t kServerPort    = 8080;
constexpr int32_t kTimeReval     = 1000;
DWORD             timeout[2]     = {static_cast<DWORD>(1000), 0};
const int         size           = sizeof(DWORD);
constexpr char    kBroadCastMode = 1;
constexpr int32_t kMaxBuffer     = 4096;
const std::string kClose         = "close";
constexpr int32_t kNoErr         = 0;
const std::string kNewConnection = "NewConnection";

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
    s.set_socketaddr(AF_INET, Controller::kDefaultPort, kLocalAddress);
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
        std::cout << "Cant not bind error code server:" << WSAGetLastError();
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
        std::cout << "Cant not bind error code client:" << WSAGetLastError();
        return false;
    }
    return true;
}

}  // namespace
int main(int32_t arcv, char** argv) {
    SocketNode s;
    SocketConnectToControl(s, kNewConnection);
    int32_t role = s.get_role();
    while (role != unknown_role_) {
        if (role == server_) {
            if (!SocketServer(s))
                break;
            std::cout << "Server is running";
            while (true) {
                std::string text = GetTime();
                s.SendTo(text.c_str());
            }
        }
        if (role == client_) {
            int32_t     res;
            std::string text(kMaxBuffer, '\0');
            std::string ip;
            std::cout << "Enter local ip: ";
            std::cin >> ip;
            if (!SocketClient(s, ip))
                break;
            do {
                res = s.RecvFrom(text);
                if (res == INVALID_SOCKET &&
                    WSAGetLastError() == WSAETIMEDOUT) {
                    SocketConnectToControl(s, kNewConnection);
                    role = s.get_role();
                } else {
                    std::cout << text.data() << std::endl;
                }
            } while (res != INVALID_SOCKET);
        }
    }
}
