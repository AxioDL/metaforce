#ifndef __DNAMP1_MAPA_HPP__
#define __DNAMP1_MAPA_HPP__

#include <vector>

#include "../DNACommon/PAK.hpp"
#include "../DNACommon/MAPA.hpp"
#include "DNAMP1.hpp"

namespace Retro
{
namespace DNAMP1
{
struct MAPA : BigDNA
{
    DECL_DNA
    Value<atUint32> magic;
    Value<atUint32> version;
    Value<atUint32> unknown1;
    Value<atUint32> unknown2;
    Value<atVec3f>  aabbMin;
    Value<atVec3f>  aabbMax;
    Value<atUint32> mappableObjectCount;
    Value<atUint32> vertexCount;
    Value<atUint32> surfaceCount;

    struct MappableObject : BigDNA
    {
        DECL_DNA
        enum class Type : atUint32
        {
            BlueDoor         = 0,
            ShieldDoor       = 1,
            IceDoor          = 2,
            WaveDoor         = 3,
            PlasmaDoor       = 4,
            BigDoor1         = 5,
            BigDoor2         = 6,
            IceDoorCeiling   = 7,
            IceDoorFloor     = 8,
            WaveDoorCeiling  = 9,
            WaveDoorFloor    = 10,
            IceDoorFloor2    = 13,
            WaveDoorFloor2   = 14,
            DownArrowYellow  = 27, /* Maintenance Tunnel */
            UpArrowYellow    = 28, /* Phazon Processing Center */
            DownArrowGreen   = 29, /* Elevator A */
            UpArrowGreen     = 30, /* Elite Control Access */
            DownArrowRed     = 31, /* Elevator B */
            UpArrowRed       = 32, /* Fungal Hall Access */
            TransportLift    = 33,
            SaveStation      = 34,
            MissileStation   = 37
        };
        Value<Type> type;
        Value<atUint32> unknown1;
        Value<atUint32> sclyId;
        Seek<DNA_COUNT(4), Athena::Current> seek1;
        Value<atVec4f>  transformMtx[3];
        Seek<DNA_COUNT(0x10), Athena::Current> seek2;
    };
    Vector<MappableObject, DNA_COUNT(mappableObjectCount)> mappableObjects;
    Vector<atVec3f, DNA_COUNT(vertexCount)> vertices;

    struct SurfaceHeader : BigDNA
    {
        DECL_DNA
        Value<atVec3f>  normal;
        Value<atVec3f>  centroid;
        Value<atUint32> polyOff;
        Value<atUint32> edgeOff;
    };

    Vector<SurfaceHeader, DNA_COUNT(surfaceCount)> surfaceHeaders;

    struct Surface : BigDNA
    {
        DECL_DNA
        Value<atUint32> primitiveCount;
        struct Primitive : BigDNA
        {
            DECL_DNA
            Value<atUint32> type;
            Value<atUint32> indexCount;
            Vector<atUint8, DNA_COUNT(indexCount)> indices;
            Align<4> align;
        };
        Vector<Primitive, DNA_COUNT(primitiveCount)> primitives;
        Value<atUint32> borderCount;
        struct Border : BigDNA
        {
            DECL_DNA
            Value<atUint32> indexCount;
            Vector<atUint8, DNA_COUNT(indexCount)> indices;
            Align<4> align;
        };
        Vector<Border, DNA_COUNT(borderCount)> borders;
    };

    Vector<Surface, DNA_COUNT(surfaceCount)> surfaces;

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        std::function<void(const HECL::SystemChar*)> fileChanged)
    {
        MAPA mapa;
        mapa.read(rs);
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        return DNAMAPA::ReadMAPAToBlender(conn, mapa, outPath, pakRouter, entry, force);
    }
};
}
}

#endif
