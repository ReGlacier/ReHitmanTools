#include <gtest/gtest.h>

#include <BM/LOC/LOCTree.h>
#include <BM/LOC/LOCTypes.h>
#include <BM/LOC/LOCTreeCompiler.h>
#include <BM/LOC/LOCTreeFactory.h>

using namespace BM::LOC;

TEST(CheckCompiler_Linker, LinkerMemoryMarkupGenericNodePlacement)
{
    /**
     * Simple case:
     * Generate tree
     *
     * /First
     * /Second
     *
     * Compile it, check index table, check values, try to decompile, check the tree state
     */

    auto root = LOCTreeFactory::Create();

    {
        {
            auto firstChild = LOCTreeFactory::Create("First", "FirstResult", root);
            root->AddChild(firstChild);
        }
        {
            auto secondChild = LOCTreeFactory::Create("Second", "SecondResult", root);
            root->AddChild(secondChild);
        }
    }

    // Check tree structure
    ASSERT_EQ(root->numChild, 2);
    ASSERT_EQ(root->numChild, root->children.size());

    // Check that our nodes are not marked up already
    ASSERT_FALSE(root->memoryMarkup.has_value());
    ASSERT_FALSE(root->children[0]->memoryMarkup.has_value());
    ASSERT_FALSE(root->children[1]->memoryMarkup.has_value());

    // Generate markup
    LOCTreeCompiler::MarkupTree(root);

    ASSERT_TRUE(root->memoryMarkup.has_value());
    ASSERT_TRUE(root->children[0]->memoryMarkup.has_value());
    ASSERT_TRUE(root->children[1]->memoryMarkup.has_value());

    // Lookup generated markup
    ASSERT_TRUE(root->memoryMarkup.value().StartsAt < root->memoryMarkup.value().EndsAt);

    const auto& rootMarkup   = root->memoryMarkup.value();
    const auto& firstMarkup  = root->children[0]->memoryMarkup.value();
    const auto& secondMarkup = root->children[1]->memoryMarkup.value();

    ASSERT_TRUE(firstMarkup.EndsAt - 1 < secondMarkup.StartsAt);
    ASSERT_TRUE(secondMarkup.EndsAt - 1 < rootMarkup.EndsAt);

    // Compile placement into raw buffer
    bool compileResult = false;
    LOCTreeCompiler::Buffer compiledBuffer;
    ASSERT_NO_THROW((compileResult = LOCTreeCompiler::Compile(compiledBuffer, root)));
    ASSERT_TRUE(compileResult);

    // Check through decompiler
    LOCTreeNode* decompiledRoot = nullptr;
    {
        // Try to decompile
        ASSERT_NO_THROW((decompiledRoot = LOCTreeNode::ReadFromMemory((char*)compiledBuffer.data(), compiledBuffer.size())));
        ASSERT_NE(decompiledRoot, nullptr);

        // If ok - check tree
        ASSERT_EQ(decompiledRoot->nodeType, root->nodeType);
        ASSERT_EQ(decompiledRoot->numChild, root->numChild);
        ASSERT_EQ(decompiledRoot->children.size(), root->children.size());

        for (int i = 0; i < decompiledRoot->numChild; i++)
        {
            auto decompiledChild = decompiledRoot->children[i];
            auto originalChild = root->children[i];

            ASSERT_EQ(decompiledChild->nodeType, originalChild->nodeType);
            ASSERT_EQ(decompiledChild->name, originalChild->name);
            ASSERT_EQ(decompiledChild->value, originalChild->value);
        }
    } // All ok for simple case

    // Free
    delete root;
    delete decompiledRoot;
}

static LOCTreeNode* CreateSampleTreeWithMultipleSubtrees()
{
    /**
     * Generate Tree Like M13
     *
     * /AllLevels
     *      /Actions
     *          /ActionKill - "Kill actor"
     *          /ActionOpenDoor - "Open Door"
     *          /ActionCloseDoor - "Close Door"
     *      /Dialogs
     *          /Talk
     *              /01 - "It's just a long-long string, nothing more"
     *              /02 - "Hello world!"
     *          /Random
     *              /01 - "It's just a random phrase!"
     * /FMVSubtitles
     *      /Intro
     *          /1018-1171 - "Welcome, Mr. Johnson, welcome.  May I take your briefcase?"
     *          /1180-1220 - "No! I'll keep it!"
     *          /1250-1507 - "I've heard interesting things about your establishment... I'd like to see what you have to offer...Preferably in the back.."
     *          /1523-1634 - "Of course, sir.  Right this way. . ."
     *      /Outro
     *          /1823-1889 - "We admire your objectivity, Rick."
     *          /1904-1933 - "Thank you"
     *          /1938-2272 - "Don't mention it. We need to get to the chaplain - 47’s ashes are going to take pride of place on my mantel. Chaplain!  Chaplain!"
     * /M13
     *      /MissionObjectives
     *          /1
     *              /Objective - "Leave NO witnesses"
     *              /ObjectiveDetail - "Everyone attending the funeral, must be eliminated"
     *              /Type - "T"
     *          /old
     *              /1
     *                  /Age - "47"
     *                  /Hair - "Grey"
     *                  /Height - "5'11 ft"
     *                  /Objective - "Kill Head of Alpha Zerox"
     *                  /ObjectiveDetail - "Cayne is from an extremely wealthy and influential family with a dynastic lineage tracing back to the Mayflower..."
     *                  /Weight - "116 lbs"
     *                  /WhoOrWhat - "Alexander Leland Cayne"
     *              /2
     *                  /Objective - "Retrieve journalist tape recorder"
     *                  /Type - "R"
     *                  /WhoOrWhat - "Tape recorder"
     *              /3
     *                  /Objective - "Escape in limo"
     *                  /Type - "E"
     *                  /WhoOrWhat - "Limo"
     */
    auto root = LOCTreeFactory::Create();

    auto AllLevels = LOCTreeFactory::Create("AllLevels", TreeNodeType::NODE_WITH_CHILDREN, root);
    auto FMVSubtitles = LOCTreeFactory::Create("FMVSubtitles", TreeNodeType::NODE_WITH_CHILDREN, root);
    auto M13 = LOCTreeFactory::Create("M13", TreeNodeType::NODE_WITH_CHILDREN, root);

    {
        auto Actions = LOCTreeFactory::Create("Actions", TreeNodeType::NODE_WITH_CHILDREN, AllLevels);
        {
            auto ActionKill = LOCTreeFactory::Create("ActionKill", "Kill actor", Actions);
            Actions->AddChild(ActionKill);
        }
        {
            auto ActionOpenDoor = LOCTreeFactory::Create("ActionOpenDoor", "Open Door", Actions);
            Actions->AddChild(ActionOpenDoor);
        }
        {
            auto ActionCloseDoor = LOCTreeFactory::Create("ActionCloseDoor", "Close Door", Actions);
            Actions->AddChild(ActionCloseDoor);
        }
        AllLevels->AddChild(Actions);

        auto Dialogs = LOCTreeFactory::Create("Dialogs", TreeNodeType::NODE_WITH_CHILDREN, AllLevels);
        {
            auto Talk = LOCTreeFactory::Create("Talk", TreeNodeType::NODE_WITH_CHILDREN, Dialogs);
            {
                auto T01 = LOCTreeFactory::Create("01", "It's just a long-long string, nothing more", Talk);
                Talk->AddChild(T01);
            }
            {
                auto T02 = LOCTreeFactory::Create("02", "Hello world!", Talk);
                Talk->AddChild(T02);
            }
            Dialogs->AddChild(Talk);
        }
        {
            auto Random = LOCTreeFactory::Create("Random", TreeNodeType::NODE_WITH_CHILDREN, Dialogs);
            {
                auto T01 = LOCTreeFactory::Create("01", "It's just a random phrase!", Random);
                Random->AddChild(T01);
            }
            Dialogs->AddChild(Random);
        }
        AllLevels->AddChild(Dialogs);
    }
    root->AddChild(AllLevels);

    {
        {
            auto Intro = LOCTreeFactory::Create("Intro", TreeNodeType::NODE_WITH_CHILDREN, FMVSubtitles);
            {
                auto T01 = LOCTreeFactory::Create("1018-1171", "Welcome, Mr. Johnson, welcome.  May I take your briefcase?", Intro);
                Intro->AddChild(T01);
            }
            {
                auto T02 = LOCTreeFactory::Create("1180-1220", "No! I'll keep it!", Intro);
                Intro->AddChild(T02);
            }
            {
                auto T03 = LOCTreeFactory::Create(
                        "1250-1507",
                        "I've heard interesting things about your establishment... I'd like to see what you have to offer...Preferably in the back..",
                        Intro);
                Intro->AddChild(T03);
            }
            {
                auto T04 = LOCTreeFactory::Create("1523-1634", "Of course, sir.  Right this way. . .", Intro);
                Intro->AddChild(T04);
            }
            FMVSubtitles->AddChild(Intro);
        }
        {
            auto Outro = LOCTreeFactory::Create("Outro", TreeNodeType::NODE_WITH_CHILDREN, FMVSubtitles);
            {
                auto T01 = LOCTreeFactory::Create("1823-1889", "We admire your objectivity, Rick.", Outro);
                Outro->AddChild(T01);
            }
            {
                auto T02 = LOCTreeFactory::Create("1904-1933", "Thank you", Outro);
                Outro->AddChild(T02);
            }
            {
                auto T03 = LOCTreeFactory::Create(
                        "1938-2272",
                        "Don't mention it. We need to get to the chaplain - 47’s ashes are going to take pride of place on my mantel. Chaplain!  Chaplain!",
                        Outro);
                Outro->AddChild(T03);
            }
            FMVSubtitles->AddChild(Outro);
        }
    }
    root->AddChild(FMVSubtitles);

    {
        {
            auto MO1 = LOCTreeFactory::Create("1", TreeNodeType::NODE_WITH_CHILDREN, M13);
            {
                auto Objective = LOCTreeFactory::Create("Objective", "Leave NO witnesses", MO1);
                MO1->AddChild(Objective);
            }
            {
                auto ObjectiveDetail = LOCTreeFactory::Create(
                        "ObjectiveDetail",
                        "Everyone attending the funeral, must be eliminated",
                        MO1);
                MO1->AddChild(ObjectiveDetail);
            }
            {
                auto Type = LOCTreeFactory::Create("Type", "T", MO1);
                MO1->AddChild(Type);
            }
            M13->AddChild(MO1);
        }
        {
            auto Old = LOCTreeFactory::Create("old", TreeNodeType::NODE_WITH_CHILDREN, M13);
            {
                auto O1 = LOCTreeFactory::Create("1", TreeNodeType::NODE_WITH_CHILDREN, Old);
                {
                    auto Age = LOCTreeFactory::Create("Age", "47", O1);
                    O1->AddChild(Age);
                }
                {
                    auto Hair = LOCTreeFactory::Create("Hair", "Grey", O1);
                    O1->AddChild(Hair);
                }
                {
                    auto Height = LOCTreeFactory::Create("Height", "5'11 ft", O1);
                    O1->AddChild(Height);
                }
                {
                    auto Objective = LOCTreeFactory::Create("Objective", "Kill Head of Alpha Zerox", O1);
                    O1->AddChild(Objective);
                }
                {
                    auto ObjectiveDetail = LOCTreeFactory::Create(
                            "ObjectiveDetail",
                            "Cayne is from an extremely wealthy and influential family with a dynastic lineage tracing back to the Mayflower...",
                            O1);
                    O1->AddChild(ObjectiveDetail);
                }
                {
                    auto Weight = LOCTreeFactory::Create("Weight", "116 lbs", O1);
                    O1->AddChild(Weight);
                }
                {
                    auto WhoOrWhat = LOCTreeFactory::Create("WhoOrWhat", "Alexander Leland Cayne", O1);
                    O1->AddChild(WhoOrWhat);
                }
                Old->AddChild(O1);
            }
            {
                auto O2 = LOCTreeFactory::Create("2", TreeNodeType::NODE_WITH_CHILDREN, Old);
                {
                    auto Objective = LOCTreeFactory::Create("Objective", "Retrieve journalist tape recorder", O2);
                    O2->AddChild(Objective);
                }
                {
                    auto Type = LOCTreeFactory::Create("Type", "R", O2);
                    O2->AddChild(Type);
                }
                {
                    auto WhoOrWhat = LOCTreeFactory::Create("WhoOrWhat", "Tape recorder", O2);
                    O2->AddChild(WhoOrWhat);
                }
                Old->AddChild(O2);
            }
            {
                auto O3 = LOCTreeFactory::Create("3", TreeNodeType::NODE_WITH_CHILDREN, Old);
                {
                    auto Objective = LOCTreeFactory::Create("Objective", "Escape in limo", O3);
                    O3->AddChild(Objective);
                }
                {
                    auto Type = LOCTreeFactory::Create("Type", "E", O3);
                    O3->AddChild(Type);
                }
                {
                    auto WhoOrWhat = LOCTreeFactory::Create("WhoOrWhat", "Limo", O3);
                    O3->AddChild(WhoOrWhat);
                }
                Old->AddChild(O3);
            }
            M13->AddChild(Old);
        }
    }
    root->AddChild(M13);

    return root;
}

static void CompareTreeWithOriginal(LOCTreeNode* r1, LOCTreeNode* r2)
{
    // Check
    ASSERT_NE(r1, nullptr);
    ASSERT_NE(r2, nullptr);
    ASSERT_NE(r1, r2);

    // Compare node types
    ASSERT_EQ(r1->nodeType, r2->nodeType);
    ASSERT_EQ(r1->IsRoot(), r2->IsRoot());
    if (!r1->IsRoot())
    {
        ASSERT_EQ(r1->name, r2->name);
    }

    ASSERT_EQ(r1->originalTypeRawData.has_value(), r2->originalTypeRawData.has_value());
    if (r1->originalTypeRawData.has_value())
    {
        const auto& otrdv1 = r1->originalTypeRawData.value();
        const auto& otrdv2 = r2->originalTypeRawData.value();
        ASSERT_EQ(otrdv1, otrdv2);
    }

    if (r1->IsData())
    {
        ASSERT_EQ(r1->IsData(), r2->IsData()); // It's actually not required because internal types should be equal, but why not?
        // Compare end points

        ASSERT_EQ(r1->name, r2->name);
        ASSERT_EQ(r1->value, r2->value);
    }
    else if (r1->IsContainer())
    {
        ASSERT_EQ(r1->IsContainer(), r2->IsContainer()); // Not required too but why not?

        // Compare names and iterate over endpoints
        ASSERT_EQ(r1->name, r2->name);
        ASSERT_EQ(r1->numChild, r2->numChild);
        ASSERT_EQ(r1->children.size(), r1->numChild);
        ASSERT_EQ(r2->children.size(), r2->numChild);

        for (int entityIndex = 0; entityIndex < r1->numChild; entityIndex++)
        {
            CompareTreeWithOriginal(r1->children[entityIndex], r2->children[entityIndex]);
        }
    }
}

TEST(CheckCompiler_Linker, SubTreeMultiTableLinking)
{
    auto root = CreateSampleTreeWithMultipleSubtrees();
    ASSERT_NE(root, nullptr);

    // Compile it
    LOCTreeCompiler::Buffer compiledTreeBuffer;
    bool compileResult = false;
    ASSERT_NO_THROW((compileResult = LOCTreeCompiler::Compile(compiledTreeBuffer, root)));
    ASSERT_TRUE(compileResult);

    // Decompile it
    LOCTreeNode* decompiledRoot = nullptr;
    ASSERT_NO_THROW((decompiledRoot = LOCTreeNode::ReadFromMemory((char*)compiledTreeBuffer.data(), compiledTreeBuffer.size())));
    ASSERT_NE(decompiledRoot, nullptr);

    // Check it
    CompareTreeWithOriginal(root, decompiledRoot);

    delete root;
    delete decompiledRoot;
}