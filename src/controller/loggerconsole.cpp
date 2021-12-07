#include "loggerconsole.h"

#include <algorithm>
#include <limits>
#include <string>
LoggerConsole::LoggerConsole() : in_buffer(std::cin), out_buffer(std::cout) {}

std::ostream& LoggerConsole::Write(const std::string& text_out) {
    out_buffer << text_out;
    return out_buffer;
}
bool LoggerConsole::SetWriteMode(
    const std::streamsize width, const char fill_character,
    const std::vector<std::ios_base::fmtflags> set_mode) {
    out_buffer.width(width);
    out_buffer.fill(fill_character);
    for_each(
        set_mode.begin(), set_mode.end(), [&](std::ios_base::fmtflags mode) {
            out_buffer.setf(mode);
        });
    return true;
}
std::istream& LoggerConsole::Read(std::string& text_int, const char delim) {
    std::getline(in_buffer, text_int, delim);
    return in_buffer;
}
bool LoggerConsole::Reset() {
    in_buffer.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    in_buffer.clear();
    out_buffer.clear();
    return true;
}
