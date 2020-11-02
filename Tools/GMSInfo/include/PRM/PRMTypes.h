#pragma once

/**
 * @credits: John "Cryect" Rittenhouse - Hitman PRM to Obj converter 0.1 (8/12/2006)
 */

#include <cstdint>

namespace ReGlacier
{
    struct PRMHeader
    {
        uint32_t ChunkPos;
        uint32_t ChunkNum;
        uint32_t ChunkPos2;
        uint32_t Zero;
    };

    struct Chunk
    {
        uint32_t Pos;         //+0x0
        uint32_t Size;        //+0x4
        uint32_t IsGeometry;  //+0x8
        uint32_t Unknown2;    //+0xC
    };

    struct Vertex1 {
        float Pos[3];
        int Unknown1;
    };

    struct Vertex2 {
        float Pos[3];
        int Unknown[2];
        float UV[2];
        int Unknown2[3];
    };

    struct Vertex3 {
        float Pos[3];
        float Unknown1[3];
        int Unknown2[3];
        float UV[2];
        int Unknown3[2];
    };

    struct Vertex4 {
        float Pos[3];
        float UV[2];
        int Unknown[2];
        float UV2[2];
    };

    enum VertexType {
        VT_1 = 0x10,
        VT_2 = 0x28,
        VT_3 = 0x34,
        VT_4 = 0x24,
        VT_UNKNOWN = 0x0
    };

    struct Descriptor {
        int UnknownNumber1;
        int VertexChunk;
        int Unknown2;
        int TriangleChunk;
    };

    struct MeshPartObject
    {
        int Unknown1[10];
        int MeshPtr;
        int Unknown2[4];
        int UnknownPointer;
    };

    struct MeshObject
    {
        int	Unknown1[3];
        int	Unknown2;
        int	PartsPointer;
        int	ObjectsCount;	//Number of objects for this model
        int	MeshPartPointer;	//ptr to the Object List which is just a list of ints that point to ModelObjects
        int	unk3;
        float Extents[2][3];	//Extents of this model
    };
}