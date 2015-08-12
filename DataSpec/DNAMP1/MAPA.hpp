#ifndef __DNAMP1_MAPA_HPP__
#define __DNAMP1_MAPA_HPP__

#include <vector>

#include "../DNACommon/DNACommon.hpp"

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
        Value<atUint32> type;
        Value<atUint32> unknown1;
        Value<atUint16> unknown2;
        Value<atUint16> id;
        Seek<DNA_COUNT(4), Athena::Current> seek1;
        Value<atVec4f>  transform1;
        Value<atVec4f>  transform2;
        Value<atVec4f>  transform3;
        Seek<DNA_COUNT(0x10), Athena::Current> seek2;
    };
    Vector<MappableObject, DNA_COUNT(mappableObjectCount)> mappableObjects;
    Vector<atVec3f, DNA_COUNT(vertexCount)> vertices;

    struct SurfaceHeader : BigDNA
    {
        DECL_DNA
        Value<atVec3f>  normal;
        Value<atVec3f>  center;
        Value<atUint32> start;
        Value<atUint32> end;
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
            Vector<atUint32, DNA_COUNT(indexCount)> indices;
            Align<4> align;
        };
        Vector<Primitive, DNA_COUNT(primitiveCount)> primitives;
        Value<atUint32> borderCount;
        struct Border : BigDNA
        {
            DECL_DNA
            Value<atUint32> indexCount;
            Vector<atUint32, DNA_COUNT(indexCount)> indices;
            Align<4> align;
        };
        Vector<Border, DNA_COUNT(borderCount)> borders;
    };

    Vector<Surface, DNA_COUNT(surfaceCount)> surfaces;
};
}
}

#endif
