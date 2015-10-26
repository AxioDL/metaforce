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
        enum Type : atUint32
        {
            MOBlueDoor         = 0,
            MOShieldDoor       = 1,
            MOIceDoor          = 2,
            MOWaveDoor         = 3,
            MOPlasmaDoor       = 4,
            MOBigDoor1         = 5,
            MOBigDoor2         = 6,
            MOIceDoorCeiling   = 7,
            MOIceDoorFloor     = 8,
            MOWaveDoorCeiling  = 9,
            MOWaveDoorFloor    = 10,
            MOIceDoorFloor2    = 13,
            MOWaveDoorFloor2   = 14,
            MODownArrowYellow  = 27, /* Maintenance Tunnel */
            MOUpArrowYellow    = 28, /* Phazon Processing Center */
            MODownArrowGreen   = 29, /* Elevator A */
            MOUpArrowGreen     = 30, /* Elite Control Access */
            MODownArrowRed     = 31, /* Elevator B */
            MOUpArrowRed       = 32, /* Fungal Hall Access */
            MOTransportLift    = 33,
            MOSaveStation      = 34,
            MOMissileStation   = 37
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
