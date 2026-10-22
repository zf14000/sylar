#include "config.h"

namespace sylar {

ConfigVarBase::ConfigVarBase(const std::string& name, const std::string& desc)
    :m_name(name)
    ,m_desc(desc) {
}

static void ListYamlNode(std::string prefix, const YAML::Node& node,
        std::list<std::pair<std::string, const YAML::Node> >& lists) {
    if (node.IsNull()) {
        return;
    } else if (node.IsScalar()) {
        lists.push_back(std::make_pair(prefix, node));
    } else if (node.IsSequence()) {
        lists.push_back(std::make_pair(prefix, node));
    } else if (node.IsMap()) {
        for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
            std::string new_prefix = prefix.empty() ?
                it->first.as<std::string>() :
                prefix + '.' + it->first.as<std::string>();
            ListYamlNode(new_prefix, it->second, lists);
        }
    }
}

void Config::LoadYaml(const std::string& file) {
    YAML::Node node = YAML::LoadFile(file);
    std::list<std::pair<std::string, const YAML::Node> > lists;
    ListYamlNode("", node, lists);
    for (auto& i : lists) {
        std::string name = i.first;
        if (name.empty()) {
            continue;
        }

        ConfigVarBase::ptr cfg = Config::LookupBase(name);
        if (cfg) {
            std::stringstream ss;
            ss << i.second;
            cfg->fromString(ss.str());
        } else {
        }
    }
}

std::string Config::ToString() {
    std::stringstream ss;
    for (auto it = GetConfMap().begin(); it != GetConfMap().end(); ++it) {
        ss << it->first << ": " << it->second->toYamlString() << std::endl;
    }
    return ss.str();
}

}