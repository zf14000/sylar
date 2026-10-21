#include "time_util.h"
#include <time.h>
#include <iostream>
#include <cstring>

namespace sylar {
namespace utils {

std::string Time::ToString(time_t t, const std::string& fmt) {
    const size_t TIME_BUF_MAX_SIZE = 128;
    std::string outstr(TIME_BUF_MAX_SIZE, '\0');

    struct tm tm_info{};
    if (::localtime_r(&t, &tm_info) == nullptr) {
        std::cerr << "localtime_r errno=" << errno << ", strerr=" << strerror(errno) << "\n";
        return nullptr;
    }

    size_t ret = strftime(outstr.data(), outstr.size(), fmt.c_str(), &tm_info);
    if (ret == 0) {
        std::cerr << "strftime format failed or buffer insufficient\n";
        return nullptr;
    }
    outstr.resize(ret);
    return outstr;
}

}
}