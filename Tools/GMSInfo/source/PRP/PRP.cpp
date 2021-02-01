#include <PRP/PRP.h>
#include <PRP/PRPTypes.h>
#include <PRP/PRPWalker.h>
#include <PRP/IPRPVisitor.h>
#include <PRP/DebugPRPVisitor.h>

#include <LevelAssets.h>
#include <LevelContainer.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <spdlog/spdlog.h>

namespace ReGlacier
{
    PRP::PRP(std::string name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets)
    {}

    PRP::~PRP()
    {
    }

    bool PRP::Load()
    {
        size_t prpBufferSize = 0;
        auto prpBuffer = m_container->Read(m_name, prpBufferSize);
        if (!prpBuffer)
        {
            spdlog::error("PRP::Load| Failed to load asset {}", m_name);
            return false;
        }

        DebugPRPVisitor visitor {};
        PRPWalker walker { std::move(prpBuffer), prpBufferSize };
        walker.Prepare(&visitor);
        return true;
    }

    void PRP::PrintInfo()
    {
    }
}
