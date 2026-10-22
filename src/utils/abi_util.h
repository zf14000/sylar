#ifndef __SYLAR_UTILS_ABI_H__
#define __SYLAR_UTILS_ABI_H__

#include <string>
#include <typeinfo>

namespace sylar {
namespace utils {

class ABI {
public:
    template<typename T>
    static std::string TypeName() {
        return Demangle(typeid(T).name());
    }

    template<typename T>
    static std::string TypeName(const T& val) {
        return Demangle(typeid(val).name());
    }
private:
    static std::string Demangle(const char* mangled_name);
};

}
}

#endif