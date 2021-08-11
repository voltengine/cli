#include "date.hpp"

namespace util {

date::sys_time<std::chrono::milliseconds>
        parse_iso_date(const std::string &str) {
    std::istringstream ss(str);
    date::sys_time<std::chrono::milliseconds> time;

    bool zulu = str.find('Z') != std::string::npos;

    if (zulu)
        ss >> date::parse("%FT%TZ", time);
    else
        ss >> date::parse("%FT%T%z", time);

    return time;
}

}
