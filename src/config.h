#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include "log.h"
#include "log_config.h"
#include "abi_util.h"
#include <yaml-cpp/yaml.h>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <cstring>
#include <list>
#include <vector>
#include <queue>
#include <set>
#include <map>

namespace sylar {

class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;
    ConfigVarBase(const std::string& name, const std::string& desc);
    virtual void fromString(const std::string& str) = 0;
    virtual std::string toYamlString() = 0;
    virtual std::string getType() const = 0;
    std::string getName() const { return m_name; }
    std::string getDesc() const { return m_desc; }
    void setName(const std::string& name) { m_name = name; }
    void setDesc(const std::string& desc) { m_desc = desc; }
private:
    std::string m_name;
    std::string m_desc;
};

template<typename F, typename T>
class LexicalCast {
public:
    T operator()(const F& f) const {
        try {
            return boost::lexical_cast<T>(f);
        } catch (const boost::bad_lexical_cast& e) {
            SYLAR_STREAM_LOG_ERROR(SYLAR_LOG_NAME("root"))
                << "lexical_cast " << sylar::utils::ABI::TypeName<F>()
                << " to " << sylar::utils::ABI::TypeName<T>()
                << " failed, what: " << e.what();
            throw std::runtime_error("lexical_cast failed: " + std::string(e.what()));
        }
    }
};

template<typename T>
class LexicalCast<std::string, std::list<T> > {
public:
    std::list<T> operator()(const std::string& str) const {
        std::list<T> vec;
        YAML::Node node = YAML::Load(str);
        for (size_t i = 0; i < node.size(); ++i) {
            std::stringstream ss;
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& vec) const {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : vec) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T>
class LexicalCast<std::string, std::vector<T> > {
public:
    std::vector<T> operator()(const std::string& str) const {
        std::vector<T> vec;
        YAML::Node node = YAML::Load(str);
        for (size_t i = 0; i < node.size(); ++i) {
            std::stringstream ss;
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& vec) const {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : vec) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T>
class LexicalCast<std::string, std::set<T> > {
public:
    std::set<T> operator()(const std::string& str) const {
        std::set<T> vec;
        YAML::Node node = YAML::Load(str);
        for (size_t i = 0; i < node.size(); ++i) {
            std::stringstream ss;
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& vec) const {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : vec) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T>
class LexicalCast<std::string, std::unordered_set<T> > {
public:
    std::unordered_set<T> operator()(const std::string& str) const {
        std::unordered_set<T> vec;
        YAML::Node node = YAML::Load(str);
        for (size_t i = 0; i < node.size(); ++i) {
            std::stringstream ss;
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& vec) const {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : vec) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator()(const std::string& str) const {
        std::map<std::string, T> vec;
        YAML::Node node = YAML::Load(str);
        for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
            std::stringstream ss;
            ss << it->second;
            vec[it->first.as<std::string>()] = LexicalCast<std::string, T>()(ss.str());
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& vec) const {
        YAML::Node node(YAML::NodeType::Map);
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            node[it->first] = YAML::Load(LexicalCast<T, std::string>()(it->second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
public:
    std::unordered_map<std::string, T> operator()(const std::string& str) const {
        std::unordered_map<std::string, T> vec;
        YAML::Node node = YAML::Load(str);
        for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
            std::stringstream ss;
            ss << it->second;
            vec[it->first.as<std::string>()] = LexicalCast<std::string, T>()(ss.str());
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& vec) const {
        YAML::Node node(YAML::NodeType::Map);
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            node[it->first] = YAML::Load(LexicalCast<T, std::string>()(it->second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<>
class LexicalCast<std::string, LoggerDefine> {
public:
    LoggerDefine operator()(const std::string& str) const {
        YAML::Node node = YAML::Load(str);
        LoggerDefine logger;

        if (!node["name"].IsDefined()) {
            throw std::runtime_error("lexical_cast string to LoggerDefine failed, log name is not defined");
        }
        logger.name = node["name"].as<std::string>();

        if (node["level"].IsDefined()) {
            std::string level = node["level"].as<std::string>();
            if (level == "UNKNOWN") {
                throw std::runtime_error("log: " + logger.name +
                    " lexical_cast string to LoggerDefine failed, log level is unknown");
            }
            logger.level = level;
        }

        if (node["formatter"].IsDefined()) {
            logger.formatter = node["formatter"].as<std::string>();
        }

        if (node["appenders"].IsDefined()) {
            for (size_t i = 0; i < node["appenders"].size(); ++i) {
                LogAppenderDefine appender;
                YAML::Node node_appender = node["appenders"][i];

                if (node_appender["level"].IsDefined()) {
                    std::string level = node_appender["level"].as<std::string>();
                    if (level == "UNKNOWN") {
                        throw std::runtime_error("log: " + logger.name +
                            " lexical_cast string to LoggerDefine failed, appender level is unknown");
                    }
                    appender.level = level;
                }

                if (node_appender["type"].IsDefined()) {
                    std::string type = node_appender["type"].as<std::string>();
                    appender.type = type;
                    if (type == "FileLogAppender") {
                        if (!node_appender["file"].IsDefined()) {
                            throw std::runtime_error("log: " + logger.name +
                                " lexical_cast string to LogAppenderDefine failed, appender file is not defined");
                        }
                        appender.file = node_appender["file"].as<std::string>();
                    }
                } else {
                    throw std::runtime_error("log: " + logger.name +
                        " lexical_cast string to LoggerDefine failed, appender type is not defined");
                }
                
                if (node_appender["formatter"].IsDefined()) {
                    appender.formatter = node_appender["formatter"].as<std::string>();
                }

                logger.appenders.push_back(appender);
            }
        }

        return logger;
    }
};

template<>
class LexicalCast<LoggerDefine, std::string> {
public:
    std::string operator()(const LoggerDefine& logger) const {
        YAML::Node node;
        node["name"] = logger.name;
        if (logger.level != "UNKNOWN") {
            node["level"] = logger.level;
        }
        if (!logger.formatter.empty()) {
            node["formatter"] = logger.formatter;
        }
        for (auto& appender : logger.appenders) {
            YAML::Node node_appender;
            if (appender.level != "UNKNOWN") {
                node_appender["level"] = appender.level;
            }
            node_appender["type"] = appender.type;
            if (appender.type == "FileLogAppender") {
                node_appender["file"] = appender.file;
            }
            if (!appender.formatter.empty()) {
                node_appender["formatter"] = appender.formatter;
            }
            node["appenders"].push_back(node_appender);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T, typename ToStr = LexicalCast<T, std::string>, typename FromStr = LexicalCast<std::string, T> >
class ConfigVar : public ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVar<T> >;
    using on_change_cb = std::function<void(const T& oldVal, const T& newVal)>;

    ConfigVar(const std::string& name, const T& value, const std::string& desc)
        :ConfigVarBase(name, desc)
        ,m_value(value) {
    }
    
    std::string toString(const T& val) const {
        return ToStr()(val);
    }

    void fromString(const std::string& str) override {
        try {
            setValue(FromStr()(str));
        } catch (const std::runtime_error& e) {
            SYLAR_STREAM_LOG_ERROR(SYLAR_LOG_NAME("root")) << "config lexical_cast failed" 
                << ", name=" << getName() << ", error=" << e.what();
        }
    }

    std::string toYamlString() override {
        return toString(getValue());
    }

    void setValue(const T& newVal) {
        const T oldVal = m_value;
        m_value = newVal;
        for (auto& f : m_cbs) {
            f(oldVal, newVal);
        }
    }

    void addListener(on_change_cb cb) {
        m_cbs.push_back(cb);
    }

    T getValue() const { return m_value; }
    std::string getType() const override { return sylar::utils::ABI::TypeName<T>(); }
private:
    T m_value;
    std::list<on_change_cb> m_cbs;
};

class Config {
public:
    using ptr = std::shared_ptr<Config>;
    using mapType = std::unordered_map<std::string, ConfigVarBase::ptr>;
    
    template<typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& defaultVal, const std::string& desc) {
        ConfigVarBase::ptr config = LookupBase(name);
        if (config) {
            typename ConfigVar<T>::ptr configVar = std::dynamic_pointer_cast<ConfigVar<T> >(config);
            if (!configVar) {
                SYLAR_STREAM_LOG_ERROR(SYLAR_LOG_NAME("root")) << "Config name=" << name
                    << " is existed, but type is mismatch."
                    << " Expeted type is " << sylar::utils::ABI::TypeName<T>()
                    << ", actual type is " << config->getType();
            } else {
                SYLAR_STREAM_LOG_DEBUG(SYLAR_LOG_NAME("root")) << "Config name=" << name
                    << " is existed"
                    << ", type=" << configVar->getType()
                    << ", value=" << configVar->toYamlString()
                    << ", description=" << configVar->getDesc();
                return configVar;
            }
            return nullptr;
        } else {
            typename ConfigVar<T>::ptr configVar = typename ConfigVar<T>::ptr(new ConfigVar<T>(name, defaultVal, desc));
            GetConfMap()[name] = configVar;
            return configVar;
        }
    }

    static ConfigVarBase::ptr LookupBase(const std::string& name) {
        auto it = GetConfMap().find(name);
        return it == GetConfMap().end() ? nullptr : it->second;
    }

    static void LoadYaml(const std::string& file);
    static std::string ToString();
private:
    static mapType& GetConfMap() {
        static mapType m_confMap;
        return m_confMap;
    }
};

}

#endif