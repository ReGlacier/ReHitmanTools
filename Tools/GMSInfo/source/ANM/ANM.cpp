#include <ANM/ANM.h>

#include <LevelAssets.h>
#include <LevelContainer.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <spdlog/spdlog.h>

namespace ReGlacier
{
    constexpr uint32_t kMagicBytes = 0x00414E4D;

    ANM::ANM(const std::string& name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets)
    {}

    bool ANM::Load()
    {
        size_t anmBufferSize = 0;
        auto anmBuffer = m_container->Read(m_name, anmBufferSize);

        if (!anmBuffer)
        {
            spdlog::error("ANM::Load| Failed to load ANM file {}", m_name);
            return false;
        }
        BinaryWalker binaryWalker(anmBuffer.get(), anmBufferSize);

        const auto gotMagicBytes = binaryWalker.Read<uint32_t>();
        const bool isValidMagicBytes = gotMagicBytes == kMagicBytes;
        if (!isValidMagicBytes)
        {
            spdlog::error("ANM::Load| Bad magic bytes! Got {:X} required {:X}", gotMagicBytes, kMagicBytes);
            return false;
        }

        return true;
    }
}