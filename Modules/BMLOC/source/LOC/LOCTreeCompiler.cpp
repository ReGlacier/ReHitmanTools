#include <BM/LOC/Internal/LOCCompilerImpl.h>
#include <BM/LOC/LOCTreeCompiler.h>
#include <stdexcept>

namespace BM::LOC
{
    bool LOCTreeCompiler::Compile(Buffer& buffer, LOCTreeNode* rootNode, LOCSupportMode supportMode)
    {
        switch (supportMode)
        {
            case LOCSupportMode::Hitman_BloodMoney:
            {
                using Impl = BM::LOC::Internal::LOCCompilerImpl<BM::LOC::LOCSupportMode::Hitman_BloodMoney>;
                if (!Impl::MarkupTree(rootNode))
                {
                    return false;
                }

                const auto& memMarkup = rootNode->memoryMarkup.value();
                buffer.resize(memMarkup.EndsAt);

                return Impl::Compile(buffer, rootNode);
            }
            case LOCSupportMode::Hitman_Contracts:
            {
                using Impl = BM::LOC::Internal::LOCCompilerImpl<BM::LOC::LOCSupportMode::Hitman_BloodMoney>;
                if (!Impl::MarkupTree(rootNode))
                {
                    return false;
                }

                const auto& memMarkup = rootNode->memoryMarkup.value();
                buffer.resize(memMarkup.EndsAt);

                return Impl::Compile(buffer, rootNode);
            }
            case LOCSupportMode::Hitman_2SA:
            case LOCSupportMode::Hitman_A47:
                return false;
        }

        return false;
    }

    void LOCTreeCompiler::MarkupTree(LOCTreeNode* rootNode)
    {
        BM::LOC::Internal::LOCCompilerImpl<BM::LOC::LOCSupportMode::Hitman_BloodMoney>::MarkupTree(rootNode);
    }
}