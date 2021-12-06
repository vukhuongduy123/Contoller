#ifndef REFACTOR_CONTROLLER_H
#define REFACTOR_CONTROLLER_H
#include <memory>
#include <vector>

#include "socketnode.h"
enum command { close_, add_, changerole_, newrole_ };

class Controller {
   private:
    SocketNode     controller_socket_;
    static int32_t num_socket_;
    WSADATA        wsadata_;

    bool Init() noexcept;

   public:
    static constexpr int32_t kDefaultPort = 4444;
    Controller() noexcept;

    Controller(SocketNode&)  = delete;
    Controller(Controller&&) = delete;
    Controller& operator=(const Controller&) = delete;
    Controller& operator=(Controller&&) = delete;

    ~Controller() = default;

    friend DWORD WINAPI ReadingThreadTCP(LPVOID);
};
#endif  // REFACTOR_CONTROLLER_H
