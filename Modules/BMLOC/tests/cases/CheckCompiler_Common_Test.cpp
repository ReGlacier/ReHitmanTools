#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <BM/LOC/LOCTree.h>
#include <BM/LOC/LOCJson.h>
#include <BM/LOC/LOCTypes.h>
#include <BM/LOC/LOCTreeFactory.h>

#include <vector>
#include <memory>

#include <cstdint>

using namespace BM::LOC;

class LOC_Compiler_Common : public ::testing::Test
{
protected:
    /**
     * Check sample localization tree
     */
    static void CheckSampleTree(const LOCTreeNode* root);

    /**
     * Generate sample localization tree :
     * /AllLevels/Actions/OpenDoor == "Open Door"
     * /AllLevels/Actions/CloseDoor == "Close Door"
     * /M01/Actions/Wakeup == "Wake Up"
     *
     * @return pointer to allocated tree
     */
    static LOCTreeNode* CreateSampleTree();

    /**
     * Check compiled sample tree by dictionary (search keys):
     * /AllLevels/Actions/OpenDoor == "Open Door"
     * /AllLevels/Actions/CloseDoor == "Close Door"
     * /M01/Actions/Wakeup == "Wake Up"
     *
     * @param buffer pointer to buffer with compiled LOC
     */
    static void CheckCompiledSampleTreViaDictionary(char* buffer);
};

TEST_F(LOC_Compiler_Common, LocSerialization)
{
    auto root = CreateSampleTree();
    ASSERT_NE(root, nullptr);

    // Check original tree
    CheckSampleTree(root);

    // Serialize to JSON
    std::string locJsonStr;
    {
        nlohmann::json locJson;
        nlohmann::adl_serializer<LOCTreeNode>::to_json(locJson, root);
        locJsonStr = locJson.dump();
    }

    // Deserialize
    LOCTreeNode* deserializedRoot = nullptr;
    {
        deserializedRoot = new LOCTreeNode(nullptr, nullptr);
        auto locJson = nlohmann::json::parse(locJsonStr);
        nlohmann::adl_serializer<LOCTreeNode>::from_json(locJson, deserializedRoot);
    }

    // Check tree
    CheckSampleTree(deserializedRoot);

    delete root;
    delete deserializedRoot;
}

TEST_F(LOC_Compiler_Common, FullCompilerCheck)
{
    // Generate sample tree
    auto root = CreateSampleTree();
    CheckSampleTree(root);

    // Compile root tree
    std::vector<uint8_t> compiledTreeBuffer {};
    bool compileResult = false;

    ASSERT_NO_THROW((compileResult = LOCTreeNode::Compile(root, compiledTreeBuffer)));
    ASSERT_TRUE(compileResult);

    // Decompile tree
    LOCTreeNode* newRoot = nullptr;
    ASSERT_NO_THROW((newRoot = LOCTreeNode::ReadFromMemory((char*)compiledTreeBuffer.data(), compiledTreeBuffer.size())));
    ASSERT_NE(newRoot, nullptr);
    CheckSampleTree(newRoot);

    // Search in binary
    CheckCompiledSampleTreViaDictionary((char*)compiledTreeBuffer.data());

    // Free memory
    delete root;
    delete newRoot;
}

void LOC_Compiler_Common::CheckSampleTree(const LOCTreeNode* root)
{
    // First checks
    ASSERT_EQ(root->numChild, 2);
    ASSERT_EQ(root->nodeType, TreeNodeType::NODE_WITH_CHILDREN);
    ASSERT_EQ(root->children.size(), root->numChild);

    // Check root nodes
    ASSERT_EQ(root->children[0]->name, "AllLevels");
    ASSERT_EQ(root->children[1]->name, "M01");
    ASSERT_EQ(root->children[0]->parent, root);
    ASSERT_EQ(root->children[1]->parent, root);

    // Check AllLevels
    ASSERT_EQ(root->children[0]->nodeType, TreeNodeType::NODE_WITH_CHILDREN);
    ASSERT_EQ(root->children[0]->numChild, 1);

    // Check AllLevels/Actions
    ASSERT_EQ(root->children[0]->children[0]->nodeType, TreeNodeType::NODE_WITH_CHILDREN);
    ASSERT_EQ(root->children[0]->children[0]->name, "Actions");
    ASSERT_EQ(root->children[0]->children[0]->numChild, 2);
    ASSERT_EQ(root->children[0]->children[0]->children.size(), root->children[0]->children[0]->numChild);
    ASSERT_EQ(root->children[0]->children[0]->parent, root->children[0]);

    // AllLevels/Actions/OpenDoor
    ASSERT_EQ(root->children[0]->children[0]->children[0]->nodeType, TreeNodeType::VALUE_OR_DATA);
    ASSERT_EQ(root->children[0]->children[0]->children[0]->name, "OpenDoor");
    ASSERT_EQ(root->children[0]->children[0]->children[0]->value, "Open Door");
    ASSERT_EQ(root->children[0]->children[0]->children[0]->parent, root->children[0]->children[0]);

    // AllLevels/Actions/CloseDoor
    ASSERT_EQ(root->children[0]->children[0]->children[1]->nodeType, TreeNodeType::VALUE_OR_DATA);
    ASSERT_EQ(root->children[0]->children[0]->children[1]->name, "CloseDoor");
    ASSERT_EQ(root->children[0]->children[0]->children[1]->value, "Close Door");
    ASSERT_EQ(root->children[0]->children[0]->children[1]->parent, root->children[0]->children[0]);

    // M01
    ASSERT_EQ(root->children[1]->nodeType, TreeNodeType::NODE_WITH_CHILDREN);
    ASSERT_EQ(root->children[1]->numChild, 1);
    ASSERT_EQ(root->children[1]->name, "M01");
    ASSERT_EQ(root->children[1]->children.size(), root->children[1]->numChild);
    ASSERT_EQ(root->children[1]->parent, root);

    // M01/Actions
    ASSERT_EQ(root->children[1]->children[0]->nodeType, TreeNodeType::NODE_WITH_CHILDREN);
    ASSERT_EQ(root->children[1]->children[0]->numChild, 1);
    ASSERT_EQ(root->children[1]->children[0]->name, "Actions");
    ASSERT_EQ(root->children[1]->children[0]->children.size(), root->children[1]->children[0]->numChild);
    ASSERT_EQ(root->children[1]->children[0]->parent, root->children[1]);

    // M01/Actions/Wakeup
    ASSERT_EQ(root->children[1]->children[0]->children[0]->nodeType, TreeNodeType::VALUE_OR_DATA);
    ASSERT_EQ(root->children[1]->children[0]->children[0]->name, "Wakeup");
    ASSERT_EQ(root->children[1]->children[0]->children[0]->value, "Wake Up");
    ASSERT_EQ(root->children[1]->children[0]->children[0]->parent, root->children[1]->children[0]);
}

LOCTreeNode* LOC_Compiler_Common::CreateSampleTree()
{
    auto root = LOCTreeFactory::Create();

    // All levels
    {
        // First child
        auto allLevels = LOCTreeFactory::Create("AllLevels", TreeNodeType::NODE_WITH_CHILDREN, root);
        {
            //Actions tree
            auto actions = LOCTreeFactory::Create("Actions", TreeNodeType::NODE_WITH_CHILDREN, allLevels);
            // Actions
            {
                auto openDoor = LOCTreeFactory::Create("OpenDoor", "Open Door", actions);
                actions->AddChild(openDoor);

                auto closeDoor = LOCTreeFactory::Create("CloseDoor", "Close Door", actions);
                actions->AddChild(closeDoor);
            }
            allLevels->AddChild(actions);
        }
        root->AddChild(allLevels);
    }

    // M01
    {
        auto m01 = LOCTreeFactory::Create("M01", TreeNodeType::NODE_WITH_CHILDREN, root);
        // Actions
        {
            auto actions = LOCTreeFactory::Create("Actions", TreeNodeType::NODE_WITH_CHILDREN, m01);

            // Wake Up
            {
                auto wakeUp = LOCTreeFactory::Create("Wakeup", "Wake Up", actions);
                actions->AddChild(wakeUp);
            }
            m01->AddChild(actions);
        }

        root->AddChild(m01);
    }

    return root;
}

class ResourceCollection
{
public:
    static char* Lookup(char* key, char* contents);
};

void LOC_Compiler_Common::CheckCompiledSampleTreViaDictionary(char* buffer)
{
    const std::unordered_map<std::string, std::string> keysAndValues = {
            // With leading slash
            { "/AllLevels/Actions/OpenDoor", "Open Door" },
            { "/AllLevels/Actions/CloseDoor", "Close Door" },
            { "/M01/Actions/Wakeup", "Wake Up" },
            // Without leading slash
            { "AllLevels/Actions/OpenDoor", "Open Door" },
            { "AllLevels/Actions/CloseDoor", "Close Door" },
            { "M01/Actions/Wakeup", "Wake Up" }
    };

    for (const auto& [key, value] : keysAndValues)
    {
        const char* got = ResourceCollection::Lookup((char*)key.c_str(), buffer);
        ASSERT_NE(got, nullptr);
        ASSERT_EQ(std::string(got), value);
    }
}

char * ResourceCollection::Lookup(char* key, char* buffer)
{
    // This is result of reverse engineering of function at 0x00464FF0 aka ResourceCollection::Lookup
    char *keyWithoutLeadingSlash; // edx
    char currentChar; // al
    char currentCharInKeyWithoutLeadingSlash; // al
    size_t newIndex; // ecx
    int currentKeyOffset; // ebx
    int keyIndex; // ebp
    int v8; // edi
    int v9; // eax
    int v10; // eax
    int v11; // esi
    int v12; // esi
    char *valuePtr; // edx
    size_t index; // [esp+10h] [ebp-Ch]
    int firstEntityKeyOffset; // [esp+14h] [ebp-8h]
    char *pChunkName; // [esp+18h] [ebp-4h]

    while (true)
    {
        /// KEY NORMALISATION
        keyWithoutLeadingSlash = key;
        if ( *key == '/' )  /// Search place where our key starts not from /
        {
            do
                currentChar = (keyWithoutLeadingSlash++)[1];
            while (currentChar == '/' );
            key = keyWithoutLeadingSlash;
        }

        currentCharInKeyWithoutLeadingSlash = *keyWithoutLeadingSlash;
        newIndex = 0;
        index = 0;

        if (*keyWithoutLeadingSlash != '/' )
        {
            do
            {
                if ( !currentCharInKeyWithoutLeadingSlash ) // If we have zero terminator -> break
                    break;

                currentCharInKeyWithoutLeadingSlash = keyWithoutLeadingSlash[newIndex++ + 1]; // save current char and increment newIndex
            }
            while (currentCharInKeyWithoutLeadingSlash != '/' ); // if our new char not slash -> continue

            index = newIndex;
        }

        /// KEY SEARCH AT THE CURRENT BRANCH
        currentKeyOffset = (unsigned __int8)*buffer; // First byte - the offset from formula firstEntryLocation(offset) = 4 * offset- 3
        keyIndex = 0;
        firstEntityKeyOffset = (unsigned __int8)*buffer;
        if (currentKeyOffset <= 0 )
            goto OnOrphanedTreeNodeDetected;

        pChunkName = &buffer[4 * currentKeyOffset - 3];
        do
        {
            v8 = (currentKeyOffset >> 1) + keyIndex; /// Divide by two (fast method) + keyIndex
            if ( v8 )
                v9 = *(int *)&buffer[4 * v8 - 3];
            else
                v9 = 0;
            if (strnicmp(&pChunkName[v9], keyWithoutLeadingSlash, newIndex) >= 0 ) // if value of first group greater or equal to our key
            {
                currentKeyOffset >>= 1; // Divide by two
            }
            else // Group name is less than our key
            {
                keyIndex = v8 + 1;
                currentKeyOffset += -1 - (currentKeyOffset >> 1);
            }

            newIndex = index;
            keyWithoutLeadingSlash = key;
        }
        while (currentKeyOffset > 0);

        /// VALUE RESOLVING
        currentKeyOffset = firstEntityKeyOffset; //Restore back? o_0
        if ( keyIndex )
            v10 = *(int*)&buffer[4 * keyIndex - 3];
        else
        OnOrphanedTreeNodeDetected:
            v10 = 0;
        v11 = v10 + 4 * currentKeyOffset - 3;
        if (keyIndex >= currentKeyOffset || strnicmp(&buffer[v11], keyWithoutLeadingSlash, newIndex) )
            return nullptr;

        /// ITERATION OVER TREE
        v12 = index + v11;
        valuePtr = &buffer[v12 + 2];
        buffer += v12 + 2;

        if ( !key[index] )
            return valuePtr - 1;

        key += index + 1;
    }
}