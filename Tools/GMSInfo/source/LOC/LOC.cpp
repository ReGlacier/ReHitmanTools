#include <LOC/LOC.h>
#include <BM/LOC/LOCJson.h>

#include <LevelAssets.h>
#include <LevelContainer.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <spdlog/spdlog.h>
#include <fstream>

namespace ReGlacier
{
    LOC::LOC(const std::string& name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets)
    {}

    LOC::~LOC()
    {
        if (m_root)
        {
            delete m_root;
            m_root = nullptr;
        }
    }

    bool LOC::Load()
    {
        if (m_root)
        {
            delete m_root;
            m_root = nullptr;
        }

        size_t locBufferSize = 0;
        auto locBuffer = m_container->Read(m_name, locBufferSize);

        if (!locBuffer)
        {
            spdlog::error("LOC::Load| Failed to read file {}", m_name);
            return false;
        }

        auto buffer = (char*)locBuffer.get();

        m_root = BM::LOC::LOCTreeNode::ReadFromMemory(buffer, locBufferSize);

        m_currentBufferSize = locBufferSize;
        m_currentBuffer = std::move(locBuffer);

        return true;
    }

    bool LOC::SaveAsJson(std::string_view filePath)
    {
        if (!m_root || m_root->IsEmpty())
        {
            spdlog::warn("LOC::SaveAsJson| Nothing to save into file {}", filePath);
            return false;
        }

        std::fstream file { filePath.data(), std::ios::out | std::ios::trunc };
        if (!file)
        {
            spdlog::error("LOC::SaveAsJson| Failed to create file {}", filePath);
            return false;
        }

        nlohmann::json j;
        nlohmann::adl_serializer<BM::LOC::LOCTreeNode>::to_json(j, *m_root);

        try {
            std::string jsonContents = j.dump(4);
            file.write(jsonContents.data(), jsonContents.size());
            file.close();
        } catch (const nlohmann::json::exception&)
        {
            spdlog::error("Decoder error! Wrong LOC input format or error in LOC-Tree.");
            file.close();
            return false;
        }
        catch (const std::exception& runtimeError)
        {
            spdlog::error("C++ Runtime error! Message: {}", runtimeError.what());
            file.close();
            return false;
        }

        return true;
    }

    bool LOC::HasTextResource(char* key, char* buffer)
    {
        return Lookup(key, buffer) != nullptr;
    }

    char* LOC::Lookup(char* key, char* buffer)
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
}