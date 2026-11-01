#include "assert_util.h"
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <execinfo.h>

namespace sylar {
namespace utils {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("root");

static void Backtrace(std::vector<std::string>& bt, int size, int skip);
static std::string Demangle(const char* str);

std::string BacktraceToString(int size, int skip) {
    std::stringstream ss;
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    for (auto& line : bt) {
        ss << Demangle(line.c_str()) << std::endl;
    }
    return ss.str();
}

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    int nptrs;
    void* buffer[size];
    char** strings;

    nptrs = backtrace(buffer, size);

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == nullptr) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "backtrace_symbols error";
        return;
    }

    for (int j = skip; j < nptrs; ++j) {
        bt.push_back(strings[j]);
    }

    free(strings);
}

std::string Demangle(const char* str) {
    if (str == nullptr) {
        return "";
    }
    std::string line(str);

    // backtrace_symbols 样例： ./sylar_test(_Zxxx+0xb8) [0x55f327b2c818]
    // 1. 找到 '('
    size_t pos_left = line.find('(');
    if (pos_left == std::string::npos) {
        return line;
    }
    // 从 '(' 后面开始找，找 mangled 起始：_Z
    size_t pos_z = line.find("_Z", pos_left + 1);
    if (pos_z == std::string::npos) {
        // 括号里没有 _Z，比如 libc.so.6(+0x7ec00)，原样返回
        return line;
    }
    // 从 _Z 往后，遇到 + 或者 ) 停止
    size_t pos_end = line.find_first_of("+)", pos_z);
    if (pos_end == std::string::npos) {
        return line;
    }

    // 截取纯 mangled 符号
    std::string mangled = line.substr(pos_z, pos_end - pos_z);

    int status = 0;
    char* dem_buf = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
    if (dem_buf != nullptr && status == 0) {
        std::string res = line.substr(0, pos_left + 1) + std::string(dem_buf) + line.substr(pos_end);
        free(dem_buf);
        return res;
    }
    // 反修饰失败，返回原始行
    return line;
}

}
}