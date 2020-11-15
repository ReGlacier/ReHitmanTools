#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <BM/LOC/LOCTree.h>
#include <BM/LOC/LOCJson.h>
#include <BM/LOC/LOCTypes.h>
#include <BM/LOC/LOCTreeFactory.h>

#include <vector>
#include <memory>

#include <cstdint>

class ResourceCollection
{
public:
    static char* Lookup(char* key, char* buffer);
};

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
        deserializedRoot = LOCTreeFactory::Create();
        auto locJson = nlohmann::json::parse(locJsonStr);
        nlohmann::adl_serializer<LOCTreeNode>::from_json(locJson, deserializedRoot);
    }

    // Check tree
    CheckSampleTree(deserializedRoot);

    delete root;
    delete deserializedRoot;
}

TEST_F(LOC_Compiler_Common, SerializeAndDeSerializeTreeWithDeadEnds)
{
    /*
     * Tree:
     *      /AllLevels
     *          /Enemies !root node without children!
     *          /Allies
     *              /1 - "Diana"
     *              /2 - "Smith"
     *
     */
    auto root = LOCTreeFactory::Create();
    auto AllLevels = LOCTreeFactory::Create("AllLevels", TreeNodeType::NODE_WITH_CHILDREN, root);
    root->AddChild(AllLevels);

    auto Enemies = LOCTreeFactory::Create("Enemies", TreeNodeType::NODE_WITH_CHILDREN, AllLevels);
    AllLevels->AddChild(Enemies);

    auto Allies = LOCTreeFactory::Create("Allies", TreeNodeType::NODE_WITH_CHILDREN, AllLevels);
    AllLevels->AddChild(Allies);

    auto AL01 = LOCTreeFactory::Create("1", "Diana", Allies);
    Allies->AddChild(AL01);

    auto AL02 = LOCTreeFactory::Create("2", "Smith", Allies);
    Allies->AddChild(AL02);

    // Serialize to json
    nlohmann::json source;

    ASSERT_NO_THROW(nlohmann::adl_serializer<LOCTreeNode>::to_json(source, root));

    // Deserialize back
    LOCTreeNode* newRoot = LOCTreeFactory::Create();
    ASSERT_NO_THROW(nlohmann::adl_serializer<LOCTreeNode>::from_json(source, newRoot));

    // Compare trees
    bool equality = false;
    ASSERT_NO_THROW((equality = LOCTreeNode::Compare(root, newRoot)));
    ASSERT_TRUE(equality);
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

TEST_F(LOC_Compiler_Common, ShortCheckPathFinder)
{
    static const char* kChild { "Child" };

    auto root = LOCTreeFactory::Create();
    auto child = LOCTreeFactory::Create("Child", "Another one", root);
    root->AddChild(child);

    // compile
    std::vector<uint8_t> compiledBuffer {};
    bool compileResult = false;

    ASSERT_NO_THROW((compileResult = LOCTreeNode::Compile(root, compiledBuffer)));
    ASSERT_TRUE(compileResult);

    // try to find exists key
    auto val = ResourceCollection::Lookup((char*)kChild, (char*)compiledBuffer.data());
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(std::string(val + 1), child->value);

    // Free
    delete root;
}

TEST_F(LOC_Compiler_Common, CheckLocFinderInDeepTree)
{
    /**
     * Tree:
     *      /AllLevels
     *          /Actions
     *              /OpenDoor = "Open this door!"
     *              /CloseDoor = "Close this door!"
     *              /SomeThirdAction = "Some cool action"
     *          /Dialogs
     *              /First = "First long text ..."
     *      /M13
     *          /SceneName = "Requiem"
     * Valid paths:
     * /AllLevels/Actions/OpenDoor
     * /AllLevels/Actions/CloseDoor
     * /AllLevels/Actions/SomeThirdAction
     * /AllLevels/Dialogs/First
     */
    auto root = LOCTreeFactory::Create();

    auto AllLevels = LOCTreeFactory::Create("AllLevels", TreeNodeType::NODE_WITH_CHILDREN, root);
    root->AddChild(AllLevels);

    auto Actions = LOCTreeFactory::Create("Actions", TreeNodeType::NODE_WITH_CHILDREN, AllLevels);
    AllLevels->AddChild(Actions);

    auto OpenDoor = LOCTreeFactory::Create("OpenDoor", "Open this door!", Actions);
    Actions->AddChild(OpenDoor);

    auto CloseDoor = LOCTreeFactory::Create("CloseDoor", "Close this door!", Actions);
    Actions->AddChild(CloseDoor);

    auto SomeThirdAction = LOCTreeFactory::Create("SomeThirdAction", "Some cool action", Actions);
    Actions->AddChild(SomeThirdAction);

    auto Dialogs = LOCTreeFactory::Create("Dialogs", TreeNodeType::NODE_WITH_CHILDREN, AllLevels);
    AllLevels->AddChild(Dialogs);

    auto First = LOCTreeFactory::Create("First", "First long text ...", Dialogs);
    Dialogs->AddChild(First);

    auto M13 = LOCTreeFactory::Create("M13", TreeNodeType::NODE_WITH_CHILDREN, root);
    root->AddChild(M13);

    auto SceneName = LOCTreeFactory::Create("SceneName", "Requiem", M13);
    M13->AddChild(SceneName);

    // Compile
    std::vector<uint8_t> compiledBuffer {};
    bool compileResult = false;

    ASSERT_NO_THROW((compileResult = LOCTreeNode::Compile(root, compiledBuffer)));
    ASSERT_TRUE(compileResult);

    static const char* kOpenDoorPath        = "/AllLevels/Actions/OpenDoor";
    static const char* kOpenDoorValue        = "Open this door!";

    static const char* kCloseDoorPath       = "/AllLevels/Actions/CloseDoor";
    static const char* kCloseDoorValue       = "Close this door!";

    static const char* kSomeThirdActionPath = "/AllLevels/Actions/SomeThirdAction";
    static const char* kSomeThirdActionValue = "Some cool action";

    static const char* kDialogsFirstPath    = "/AllLevels/Dialogs/First";
    static const char* kDialogsFirstValue    = "First long text ...";

    {
        char* result = ResourceCollection::Lookup((char*)kOpenDoorPath, (char*)compiledBuffer.data()); //OK
        EXPECT_NE(result, nullptr);
        if (result != nullptr)
        {
            EXPECT_EQ(std::string(result + 1), std::string(kOpenDoorValue));
        }
    }
//
    {
        char* result = ResourceCollection::Lookup((char*)kCloseDoorPath, (char*)compiledBuffer.data()); //OK
        EXPECT_NE(result, nullptr);
        if (result != nullptr)
        {
            EXPECT_EQ(std::string(result + 1), std::string(kCloseDoorValue));
        }
    }

    {
        char* result = ResourceCollection::Lookup((char*)kSomeThirdActionPath, (char*)compiledBuffer.data()); //OK
        EXPECT_NE(result, nullptr);
        if (result != nullptr)
        {
            EXPECT_EQ(std::string(result + 1), std::string(kSomeThirdActionValue));
        }
    }

    {
        char* result = ResourceCollection::Lookup((char*)kDialogsFirstPath, (char*)compiledBuffer.data()); //OK
        EXPECT_NE(result, nullptr);
        if (result != nullptr)
        {
            EXPECT_EQ(std::string(result + 1), std::string(kDialogsFirstValue));
        }
    }

    delete root;
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

    // AllLevels/Actions/CloseDoor
    ASSERT_EQ(root->children[0]->children[0]->children[0]->nodeType, TreeNodeType::VALUE_OR_DATA);
    ASSERT_EQ(root->children[0]->children[0]->children[0]->name, "CloseDoor");
    ASSERT_EQ(root->children[0]->children[0]->children[0]->value, "Close Door");
    ASSERT_EQ(root->children[0]->children[0]->children[0]->parent, root->children[0]->children[0]);

    // AllLevels/Actions/OpenDoor
    ASSERT_EQ(root->children[0]->children[0]->children[1]->nodeType, TreeNodeType::VALUE_OR_DATA);
    ASSERT_EQ(root->children[0]->children[0]->children[1]->name, "OpenDoor");
    ASSERT_EQ(root->children[0]->children[0]->children[1]->value, "Open Door");
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

void LOC_Compiler_Common::CheckCompiledSampleTreViaDictionary(char* buffer)
{
    static const char* kPath1 = "/AllLevels/Actions/OpenDoor";
    static const char* kPath2 = "/AllLevels/Actions/CloseDoor";
    static const char* kPath3 = "/M01/Actions/Wakeup";

    const char* kValue1 = ResourceCollection::Lookup((char*)kPath1, buffer);
    EXPECT_NE(kValue1, nullptr);
    if (kValue1 != nullptr)
    {
        EXPECT_EQ(std::string(kValue1 + 1), std::string("Open Door"));
    }

    const char* kValue2 = ResourceCollection::Lookup((char*)kPath2, buffer);
    EXPECT_NE(kValue2, nullptr);
    if (kValue2 != nullptr)
    {
        EXPECT_EQ(std::string(kValue2 + 1), std::string("Close Door"));
    }

    const char* kValue3 = ResourceCollection::Lookup((char*)kPath3, buffer);
    EXPECT_NE(kValue3, nullptr);
    if (kValue3 != nullptr)
    {
        EXPECT_EQ(std::string(kValue3 + 1), std::string("Wake Up"));
    }
}

char* ResourceCollection::Lookup(char* key, char* buffer)
{
    // This is result of reverse engineering of function at 0x00464FF0 aka ResourceCollection::Lookup
    char *keyWithoutLeadingSlash; // edx
    char currentChar; // al
    char currentCharInKeyWithoutLeadingSlash; // al
    size_t newIndex; // ecx
    int numChild; // ebx
    int keyIndex; // ebp
    int v8; // edi
    int v9; // eax
    int v10; // eax
    int v11; // esi
    int v12; // esi
    char *valuePtr; // edx
    size_t index; // [esp+10h] [ebp-Ch]
    int numChildOrg; // [esp+14h] [ebp-8h]
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
        numChild = (unsigned __int8)*buffer;
        keyIndex = 0;
        numChildOrg = (unsigned __int8)*buffer;
        if (numChild <= 0 )
            goto OnOrphanedTreeNodeDetected;

        pChunkName = &buffer[4 * numChild - 3];
        do
        {
            v8 = (numChild >> 1) + keyIndex;
            if ( v8 )
                v9 = *(int *)&buffer[4 * v8 - 3];
            else
                v9 = 0;

            int ret = 0;
            if ((ret = strnicmp(&pChunkName[v9], keyWithoutLeadingSlash, newIndex)) >= 0) // if value of first group greater or equal to our key
            {
                numChild >>= 1; // Divide by two
            }
            else // Group name is less than our key
            {
                keyIndex = v8 + 1;
                numChild += -1 - (numChild >> 1);
            }

            newIndex = index;
            keyWithoutLeadingSlash = key;
        }
        while (numChild > 0);

        /// VALUE RESOLVING
        numChild = numChildOrg; //Restore back? o_0
        if ( keyIndex )
            v10 = *(int*)&buffer[4 * keyIndex - 3];
        else
        OnOrphanedTreeNodeDetected:
            v10 = 0;
        v11 = v10 + 4 * numChild - 3;

        int ret = 0;
        if (keyIndex >= numChild || (ret = strnicmp(&buffer[v11], keyWithoutLeadingSlash, newIndex)))
        {
            return nullptr;
        }

        /// ITERATION OVER TREE
        v12 = index + v11;
        valuePtr = &buffer[v12 + 2];
        buffer += v12 + 2;

        if ( !key[index] )
        {
            return valuePtr - 1;
        }

        key += index + 1;
    }
}