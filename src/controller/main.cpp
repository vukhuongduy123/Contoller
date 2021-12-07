#include <iostream>
#include "controller.h"
#include "socket.h"
namespace {
const std::string kNoIp = "0.0.0.0";
}
int main(int32_t argc, char** argv) {
    if (argc != 2) {
        std::cout << "Invalid argument";
        return -1;
    }
    std::string role = argv[1];
    std::string ip;
    Controller  control(role);
    if (role == "server") {
        ip = kNoIp;
    } else if (role == "client") {
        std::cout << "enter local ip: " << std::endl;
        std::cin >> ip;
    }
    int res = control.Run(ip);
    return res;
}