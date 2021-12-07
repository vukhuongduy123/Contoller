#ifndef CONTROLLER_CONTROLLER_CONTROLLER_H
#define CONTROLLER_CONTROLLER_CONTROLLER_H
#include "socket.h"
#include "loggerconsole.h"
class Controller {
    Socket socket_node_;
    LoggerConsole logger;

   public:
    Controller(const Controller&) = delete;
    Controller(Controller&&)      = delete;
    ~Controller()                 = default;
    Controller& operator=(const Controller) = delete;
    Controller& operator=(Controller&&) = delete;
    Controller(const std::string& role) noexcept;

    int32_t Run(const std::string& ip) noexcept;
};

#endif  // CONTROLLER_CONTROLLER_CONTROLLER_H
