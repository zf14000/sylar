#ifndef __SYLAR_UTILS_SINGLETON_H__
#define __SYLAR_UTILS_SINGLETON_H__

namespace sylar {
namespace utils {

template<typename T>
class Singleton {
public:
    static T& GetInstance() {
        static T instance;
        return instance;
    }
};

}
}

#endif