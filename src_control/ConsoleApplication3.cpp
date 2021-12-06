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

}  // namespace
int main(int32_t arcv, char** argv) {
    Controller control;
}
