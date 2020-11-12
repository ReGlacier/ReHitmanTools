#pragma once

#include <nlohmann/json.hpp>
#include <LOC/LOCTree.h>

namespace nlohmann
{
    template <>
    struct adl_serializer<ReGlacier::TreeNodeType>
    {
        static constexpr const char* kValOrData = "value";
        static constexpr const char* kNode = "node";

        static void to_json(json& j, const ReGlacier::TreeNodeType& type)
        {
            switch (type)
            {
                case ReGlacier::TreeNodeType::VALUE_OR_DATA: j = kValOrData; break;
                case ReGlacier::TreeNodeType::NODE_WITH_CHILDREN: j = kNode; break;
                default: j = static_cast<int>(type); break;
            }
        }

        static void from_json(const json& j, ReGlacier::TreeNodeType& r)
        {
            if (j == kValOrData) r = ReGlacier::TreeNodeType::VALUE_OR_DATA;
            else if (j == kNode) r = ReGlacier::TreeNodeType::NODE_WITH_CHILDREN;
            else r = static_cast<ReGlacier::TreeNodeType>(j.get<int>());
        }
    };

    template <>
    struct adl_serializer<ReGlacier::LOCTreeNode>
    {
        static void to_json(json& j, const ReGlacier::LOCTreeNode& node)
        {
            switch (node.nodeType)
            {
                case ReGlacier::VALUE_OR_DATA:
                    j["name"] = node.name;
                    adl_serializer<ReGlacier::TreeNodeType>::to_json(j["type"], node.nodeType);
                    if (node.value)
                    {
                        // If not string rewrite to byte export (calc length between next node in this level or in parent' level)
                        std::string_view value { node.value };
                        j["value"] = value;
                    }
                    break;
                case ReGlacier::NODE_WITH_CHILDREN:
                    if (!node.IsRoot()) { j["name"] = node.name; }
                    assert(node.numChild == node.children.size());

                    j["numChildren"] = node.numChild;

                    for (const auto& child : node.children)
                    {
                        json jc;
                        nlohmann::adl_serializer<ReGlacier::LOCTreeNode>::to_json(jc, *child);
                        j["children"].push_back(jc);
                    }
                    break;
            }
        }

        static void from_json(const json&, ReGlacier::LOCTreeNode& opt)
        {
            assert(false); //NOT IMPLEMENTED!
        }
    };
}