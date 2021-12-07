#ifndef CONTROLLER_CONTROLLER_LOGGERCONSOLE_H
#define CONTROLLER_CONTROLLER_LOGGERCONSOLE_H
#include <iostream>
#include <vector>
class LoggerConsole {
   private:
    std::istream& in_buffer;
    std::ostream& out_buffer;

   public:
    ~LoggerConsole()                    = default;
    LoggerConsole(const LoggerConsole&) = delete;
    LoggerConsole(LoggerConsole&&)      = delete;
    LoggerConsole& operator=(const LoggerConsole) = delete;
    LoggerConsole& operator=(LoggerConsole&&) = delete;
    LoggerConsole();

    std::ostream& Write(const std::string& text);
    bool SetWriteMode(const std::streamsize width, const char fill_character,
                      const std::vector<std::ios_base::fmtflags> set_mode);
    std::istream& Read(std::string& text_int, const char delim);
    bool          Reset();
};

#endif  // !CONTROLLER_CONTROLLER_LOGGERCONSOLE_H
