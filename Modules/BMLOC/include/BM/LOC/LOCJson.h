#pragma once


#include <nlohmann/json.hpp>

#include <BM/LOC/LOCTypes.h>
#include <BM/LOC/LOCTree.h>

namespace nlohmann
{
    template <>
    struct adl_serializer<BM::LOC::HBM_MissionObjectiveType>
    {
        static void to_json(json& j, const BM::LOC::HBM_MissionObjectiveType& type)
        {
            switch (type)
            {
                case BM::LOC::HBM_Target:
                case BM::LOC::HBM_Retrieve:
                case BM::LOC::HBM_Escape:
                case BM::LOC::HBM_Dispose:
                case BM::LOC::HBM_Protect:
                case BM::LOC::HBM_Optional:
                    j = static_cast<char>(type);
                    break;
                default:
                    j = static_cast<uint8_t>(type);
                    break;
            }
        }

        static void from_json(const json& j, BM::LOC::HBM_MissionObjectiveType& r)
        {
            if (j.is_string())
            {
                r = static_cast<BM::LOC::HBM_MissionObjectiveType>(j.get<std::string>()[0]);
            }
            else
            {
                r = static_cast<BM::LOC::HBM_MissionObjectiveType>(j.get<uint8_t>());
            }
        }
    };

    template <>
    struct adl_serializer<BM::LOC::TreeNodeType>
    {
        static constexpr const char* kValOrData = "data";
        static constexpr const char* kNode = "node";

        static void to_json(json& j, const BM::LOC::TreeNodeType& type)
        {
            switch (type)
            {
                case BM::LOC::TreeNodeType::VALUE_OR_DATA: j = kValOrData; break;
                case BM::LOC::TreeNodeType::NODE_WITH_CHILDREN: j = kNode; break;
                default: j = static_cast<int>(type); break;
            }
        }

        static void from_json(const json& j, BM::LOC::TreeNodeType& r)
        {
            if (j == kValOrData) r = BM::LOC::TreeNodeType::VALUE_OR_DATA;
            else if (j == kNode) r = BM::LOC::TreeNodeType::NODE_WITH_CHILDREN;
            else r = static_cast<BM::LOC::TreeNodeType>(j.get<int>());
        }
    };

    template <>
    struct adl_serializer<BM::LOC::LOCTreeNode>
    {
        static constexpr const char* kNameToken = "name";
        static constexpr const char* kValueToken = "value";
        static constexpr const char* kTypeToken = "type";
        static constexpr const char* kNumChildrenToken = "numChildren";
        static constexpr const char* kChildrenListToken = "children";
        static constexpr const char* kOriginalTypeByteToken = "org_tbyte";

        static void to_json(json& j, const BM::LOC::LOCTreeNode* node)
        {
            adl_serializer<BM::LOC::TreeNodeType>::to_json(j[kTypeToken], node->nodeType);

            switch (node->nodeType)
            {
                case BM::LOC::VALUE_OR_DATA:
                    j[kNameToken] = node->name;

                    if (!node->value.empty())
                    {
                        // If not string rewrite to byte export (calc length between next node in this level or in parent' level)
                        std::string_view value { node->value };
                        j[kValueToken] = value;
                    }

                    if (node->originalTypeRawData.has_value()) // our value was overridden, need to save original byte
                    {
                        j[kOriginalTypeByteToken] = node->originalTypeRawData.value();
                    }
                    break;
                case BM::LOC::NODE_WITH_CHILDREN:
                    if (!node->IsRoot()) { j[kNameToken] = node->name; }
                    assert(node->numChild == node->children.size());

                    j[kNumChildrenToken] = node->numChild;

                    for (const auto& child : node->children)
                    {
                        json jc;
                        nlohmann::adl_serializer<BM::LOC::LOCTreeNode>::to_json(jc, child);
                        j[kChildrenListToken].push_back(jc);
                    }
                    break;
            }
        }

        static void from_json(const json& j, BM::LOC::LOCTreeNode* node)
        {
            node->currentBufferPtr = nullptr;

            if (auto nameIterator = j.find(kNameToken); nameIterator != j.end())
            {
                node->name = nameIterator->get<std::string>();
            }
            else if (!node->IsRoot())
            {
                throw std::exception { "Not allowed to store non-root anonymous node!" };
            }

            if (auto nodeTypeIter = j.find(kTypeToken); nodeTypeIter != j.end())
            {
                nlohmann::adl_serializer<BM::LOC::TreeNodeType>::from_json(j[kTypeToken], node->nodeType);
            }
            else
            {
                throw std::exception { "Each node should contain type!" };
            }

            switch (node->nodeType)
            {
                case BM::LOC::VALUE_OR_DATA:
                    if (auto valueIterator = j.find(kValueToken); valueIterator != j.end())
                    {
                        node->value = valueIterator->get<std::string>();
                    } else throw std::exception { "Key 'value' not found for VALUE node!" };

                    if (auto originalByteIterator = j.find(kOriginalTypeByteToken); originalByteIterator != j.end())
                    {
                        node->originalTypeRawData = originalByteIterator->get<uint8_t>();
                    }
                break;
                case BM::LOC::NODE_WITH_CHILDREN:
                {
                    if (auto numChildIterator = j.find(kNumChildrenToken); numChildIterator != j.end())
                    {
                        node->numChild = numChildIterator->get<size_t>();
                        node->children.reserve(node->numChild);
                    }
                    else throw std::exception { "Key 'numChild' not found for NWC node!" };

                    if (auto childrenIterator = j.find(kChildrenListToken); childrenIterator != j.end())
                    {
                        if (!childrenIterator->is_array())
                        {
                            throw std::exception { "Key 'children' not array for NWC node!" };
                        }

                        const int requiredNumChild = node->numChild;

                        for (int i = 0; i < requiredNumChild; i++)
                        {
                            auto child = new BM::LOC::LOCTreeNode(node, nullptr);

                            try
                            {
                                auto jsonChild = childrenIterator->at(i);
                                nlohmann::adl_serializer<BM::LOC::LOCTreeNode>::from_json(jsonChild, child);
                                node->AddChild(child);
                            }
                            catch (const nlohmann::json::out_of_range& outOfRange)
                            {
                                throw std::exception { "Bad 'numChild' value in json for NWC node!" };
                            }
                        }
                    } //Allowed to have 0 child nodes
                }
                break;
                default:
                {
                    throw std::exception { "Unknown node type!" };
                }
                break;
            }
        }
    };
}