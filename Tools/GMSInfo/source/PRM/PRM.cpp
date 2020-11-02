#include <PRM/PRM.h>
#include <LevelAssets.h>
#include <LevelContainer.h>
#include <PRM/PRMTypes.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>

#include <spdlog/spdlog.h>

#include <utility>
#include <fstream>

namespace ReGlacier
{
    static constexpr size_t kMeshSize = 0x40;

    PRM::PRM(std::string name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets) {}

    bool PRM::Load()
    {
        size_t prmBufferSize = 0;
        auto prmBuffer = m_container->Read(m_assets->PRM, prmBufferSize);

        if (!prmBuffer)
        {
            spdlog::error("PRM::Load| Failed to load PRM file {}", m_assets->PRM);
            return false;
        }

        auto header = reinterpret_cast<PRMHeader*>(prmBuffer.get());
        auto chunks = reinterpret_cast<Chunk*>(prmBuffer.get() + header->ChunkPos);

        for (int meshIndex = 0; meshIndex < header->ChunkNum; ++meshIndex)
        {
            LoadMesh(meshIndex, (char*)prmBuffer.get(), prmBufferSize);
        }

        return true;
    }

    bool PRM::LoadMesh(int meshIndex, char* prm, size_t prmBufferSize)
    {
        auto header = reinterpret_cast<PRMHeader*>(prm);
        auto chunks = reinterpret_cast<Chunk*>(prm + header->ChunkPos);

        MeshObject* meshObject = nullptr;

        if (chunks[meshIndex].Size != kMeshSize)
        {
            spdlog::warn("PRM::LoadMesh| Mesh #{} ignored. Invalid size (required {:X} ; declared {:X})",
                         meshIndex, kMeshSize, chunks[meshIndex].Size);
            return false;
        }

        meshObject = reinterpret_cast<MeshObject*>(prm + chunks[meshIndex].Pos);

        if (meshObject->PartsPointer >= header->ChunkNum)
        {
            spdlog::warn("PRM::LoadMesh| Mesh #{} ignored. Declared parts pointer is out of bounds", meshIndex);
            return false;
        }

        if (meshObject->MeshPartPointer >= header->ChunkNum)
        {
            spdlog::warn("PRM::LoadMesh| Mesh #{} ignored. Declared mesh part pointer is out of bounds",
                         meshIndex);
            return false;
        }

        bool validExtents = true;

        for (int i = 0; i < 3; i++)
        {
            if (meshObject->Extents[0][i] > meshObject->Extents[1][i])
            {
                validExtents=false;
                break;
            }
        }

        if (!validExtents)
        {
            spdlog::warn("PRM::LoadMesh| Mesh #{} ignored. Mesh extends not valid", meshIndex);
            return false;
        }

        if (meshObject->ObjectsCount * 4 > chunks[meshObject->MeshPartPointer].Size)
        {
            spdlog::warn("PRM::LoadMesh| Mesh #{} ignored. "
                         "Declared objects count is out of bounds in chunks buffer", meshIndex);
            return false;
        }

        return true;
//      NOT REQUIRED!
//        auto objects = reinterpret_cast<uint32_t*>(chunks[meshObject->MeshPartPointer].Pos + prm);
//        int triangleOffset = 1;
//
//        for (int partIndex = 0; partIndex <  meshObject->ObjectsCount; ++partIndex)
//        {
//            MeshPartObject* part = nullptr;
//            unsigned int check = 0;
//            int vertexSize = 0;
//
//            if (objects[partIndex] >= header->ChunkNum)
//            {
//                spdlog::warn("PRM::LoadMesh| Mesh #{} part {} ignored. Object reference is out of bounds", meshIndex, partIndex);
//                continue;
//            }
//
//            part = reinterpret_cast<MeshPartObject*>(prm + chunks[objects[partIndex]].Pos);
//
//            if (part->MeshPtr >= header->ChunkNum)
//            {
//                spdlog::warn("PRM::LoadMesh| Mesh #{} part {} ignored. Mesh pointer is out of bounds", meshIndex, partIndex);
//                continue;
//            }
//
//            check = *reinterpret_cast<uint32_t*>(prm + chunks[part->MeshPtr].Pos);
//
//            if (check >= header->ChunkNum)
//            {
//                spdlog::warn("PRM::LoadMesh| Mesh #{} part {} ignored. Mesh pointer is out of bounds", meshIndex, partIndex);
//                continue;
//            }
//
//            Descriptor* descriptor = nullptr;
//            uint16_t* trianglesNumber;
//            uint16_t* triangles;
//
//            descriptor = reinterpret_cast<Descriptor*>(prm + chunks[check].Pos);
//
//            if (descriptor->UnknownNumber1 == 0)
//            {
//                spdlog::warn("PRM::LoadMesh| Mesh #{} part {} ignored. "
//                             "Invalid number at UnknownNumber1 region. Await 0 but got {}",
//                             meshIndex,
//                             partIndex,
//                             descriptor->UnknownNumber1);
//                continue;
//            }
//
//            trianglesNumber = reinterpret_cast<uint16_t*>(prm + chunks[descriptor->TriangleChunk].Pos + 0x2);
//            triangles = reinterpret_cast<uint16_t*>(prm + chunks[descriptor->TriangleChunk].Pos + 0x4);
//
//            vertexSize = chunks[descriptor->VertexChunk].Size / descriptor->UnknownNumber1;
//            vertexSize -= vertexSize % 4;
//
//            /**
//             * @brief (Author's comment): Not sure how this happens exactly but sometimes its a multiple?
//             */
//            if ((vertexSize != 0x28) || ((vertexSize % 0x28) == 0))
//            {
//                descriptor->UnknownNumber1 *= vertexSize / 0x28;
//                vertexSize = 0x28;
//            }
//
//            auto vertexType = static_cast<VertexType>(vertexSize);
//            switch (vertexType)
//            {
//                case VT_1:
//                {
//                    auto vertex = reinterpret_cast<Vertex1*>(prm + chunks[descriptor->VertexChunk].Pos);
//
//                }
//                    break;
//                case VT_2:
//                {
//
//                }
//                    break;
//                case VT_3:
//                {
//
//                }
//                    break;
//                case VT_4:
//                {
//
//                }
//                    break;
//                case VT_UNKNOWN:
//                {
//                    spdlog::warn("PRM::LoadMesh| Mesh #{} part #{} ignored. Unknown vertex format", meshIndex, partIndex);
//                    continue;
//                }
//                break;
//                default:
//                {
//                    spdlog::warn("PRM::LoadMesh| Mesh #{} part #{} ignored. "
//                                 "Chunk {} has a vertex size of {:X} (possible mesh instance?) {}",
//                                 meshIndex, partIndex, descriptor->VertexChunk,
//                                 vertexSize, chunks[descriptor->VertexChunk].Unknown1);
//                    continue;
//                }
//                break;
//            }
//        }
    }
}