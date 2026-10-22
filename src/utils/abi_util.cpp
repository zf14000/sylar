#include "abi_util.h"
#include <memory>
#include <cxxabi.h>

namespace sylar {
namespace utils {

std::string ABI::Demangle(const char* mangled_name) {
    int status = 0;
    std::unique_ptr<char, decltype(&free)> demangled(
        abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status),
        free
    );
    if (status == 0 && demangled != nullptr) {
        return std::string(demangled.get());
    }
    return mangled_name;
}

}
}