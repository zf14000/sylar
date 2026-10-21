#ifndef __SYLAR_UTILS_TIME_H__
#define __SYLAR_UTILS_TIME_H__

#include <string>

namespace sylar {
namespace utils {

class Time {
public:
    static std::string ToString(time_t t, const std::string& fmt);
};

}
}

#endif