#include "log_config.h"
#include "config.h"

void test() {
    // SYLAR_STREAM_LOG_DEBUG(SYLAR_LOG_NAME("root")) << "=============before load=============";
    // SYLAR_STREAM_LOG_INFO(SYLAR_LOG_NAME("root")) << sylar::Config::ToString();

    // const std::string file = "/home/zf/code/sylar/conf/log_config.yml";
    // sylar::Config::LoadYaml(file);

    SYLAR_STREAM_LOG_DEBUG(SYLAR_LOG_NAME("system")) << "=============after load=============";
    SYLAR_STREAM_LOG_INFO(SYLAR_LOG_NAME("system")) << sylar::Config::ToString();
    SYLAR_STREAM_LOG_INFO(SYLAR_LOG_NAME("root")) << sylar::Config::ToString();
}

int main() {
    test();
    return 0;
}