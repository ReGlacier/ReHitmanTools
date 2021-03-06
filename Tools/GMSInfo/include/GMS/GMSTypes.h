#pragma once

#include <cstdint>

#include <GlacierTypeDefs.h>

namespace ReGlacier
{
    struct GMSGeomReference
    {
        union TD_t {
            int32_t rawTypeId;
            Glacier::TypeId typeId;
        } type;
        int32_t id;
        int32_t unknown;
    };

    struct GMSLinkRef
    {
        uint32_t index = 0u;

        struct {
            uint32_t index = 0;
            Glacier::TypeId id = Glacier::NOT_INITIALISED;
        } typeInfo;

        GMSLinkRef() = default;
        GMSLinkRef(uint32_t _index, uint32_t _typeId, bool isDeclared)
                : index(_index)
        {
            typeInfo.index = _typeId;
            typeInfo.id = isDeclared ? static_cast<Glacier::TypeId>(_typeId) : Glacier::NOT_FOUND;
        }
    };

    struct GMSWeaponHandle
    {
        uint32_t entityId = 0;
        uint32_t m_field4 = 0;
        uint32_t m_field8 = 0;

        GMSWeaponHandle() = default;
        GMSWeaponHandle(uint32_t id, uint32_t u0, uint32_t u1)
                : entityId(id), m_field4(u0), m_field8(u1) {}
    };

    /// -------------------------------------------------
    struct SGMSUncompressedHeader
    {
        uint32_t TotalEntitiesCountPos; //0x0
        uint32_t Unknown4;  //0x4
        uint32_t Unknown8;  //0x8
        uint32_t UnknownC;  //0xC
        uint32_t DataPos;   //0x10
        uint32_t Unknown14;  //0x14
        uint32_t Unknown18;  //0x18
        uint32_t Unknown1C;  //0x1C
        uint32_t Unknown20;  //0x20
        uint32_t Unknown24;  //0x24
        uint32_t Unknown28; //0x28
        uint32_t Unknown2C; //0x2C
        uint32_t Unknown30; //0x30
        uint32_t Unknown34; //0x34
        uint32_t Unknown38; //0x38
        uint32_t BaseGeomsCount; //0x3C
    };

    struct SGMSEntry
    {
        uint32_t Unknown1;
        uint32_t TypeInfoPos;
    };

    struct SGMSBaseGeom
    {
        uint32_t PrimitiveBufGroupNameOffset; //+0x0
        uint32_t Unknown4; //+0x4
        uint32_t Unknown8; //+0x8
        uint32_t UnknownC; //+0xC
        uint32_t Unknown10; //+0x10
        Glacier::TypeId TypeId; //+0x14
        uint32_t Unknown18; //+0x18
        uint32_t Unknown1C; //+0x1C
        uint32_t PRMOffset; //+0x20
        uint32_t Unknown24; //+0x24
        uint32_t Unknown28; //+0x28;
        uint32_t Unknown2C; //+0x2C
        uint32_t Unknown30; //+0x30
        uint32_t Unknown34; //+0x34
        uint32_t Unknown38; //+0x38
        uint32_t Unknown3C; //+0x3C
    };

    struct GMSComposedInfoHolder
    {
        int id;
        SGMSBaseGeom baseGeom;
        std::string  groupName;
    };
}