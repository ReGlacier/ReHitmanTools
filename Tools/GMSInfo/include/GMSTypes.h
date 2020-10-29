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

    struct SPackedGeomsHeader
    {
        int m_field0;
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
        int m_field20;
        int m_field24;
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
}