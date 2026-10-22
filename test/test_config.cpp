#include "config.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("root");

static sylar::ConfigVarBase::ptr g_conf_arr[] = {
    sylar::Config::Lookup("value.int", (int)1, "value int"),
    sylar::Config::Lookup("value.float", (float)1.1, "value float"),
    sylar::Config::Lookup("value.double", (double)3.1415926, "value double"),
    sylar::Config::Lookup("value.string", std::string("%Y/%m/%d"), "value string"),
    sylar::Config::Lookup("value.list", std::list<int>{1, 3, 2, 4}, "value list"),
    sylar::Config::Lookup("value.vector", std::vector<int>{1, 3, 2, 4}, "value vector"),
    sylar::Config::Lookup("value.set", std::set<int>{2, 2, 3, 4, 1}, "value set"),
    sylar::Config::Lookup("value.unordered_set", std::unordered_set<int>{2, 2, 3, 4, 1}, "value unordered_set"),
    sylar::Config::Lookup("value.map", std::map<std::string, std::string>{
        std::make_pair<std::string, std::string>("key2", "value2"),
        std::make_pair<std::string, std::string>("key3", "value3"),
        std::make_pair<std::string, std::string>("key1", "value1"),
        std::make_pair<std::string, std::string>("key4", "value4"),
    }, "value map"),
    sylar::Config::Lookup("value.unordered_map", std::unordered_map<std::string, std::string>{
        std::make_pair<std::string, std::string>("key2", "value2"),
        std::make_pair<std::string, std::string>("key3", "value3"),
        std::make_pair<std::string, std::string>("key1", "value1"),
        std::make_pair<std::string, std::string>("key4", "value4"),
    },"value unordered_map")
};

void print_config() {
    SYLAR_STREAM_LOG_INFO(g_logger) << sylar::Config::ToString();
}

void test_config() {
    print_config();
    sylar::ConfigVar<float>::ptr int_val_err = sylar::Config::Lookup("value.int", (float)1.1, "value int");
}

void test_yaml() {
    const std::string file = "/home/zf/code/sylar/conf/test_config.yml";
    sylar::Config::LoadYaml(file);
    print_config();
}

int main() {
    test_config();
    SYLAR_STREAM_LOG_INFO(g_logger) << "======================";
    test_yaml();
    return 0;
}