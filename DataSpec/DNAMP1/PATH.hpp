#ifndef __DNACOMMON_PATH_HPP__
#define __DNACOMMON_PATH_HPP__

#include "../DNACommon/DNACommon.hpp"

namespace DataSpec
{
struct PATH : BigDNA
{
    DECL_DNA
    Value<atUint32> version;
    struct Vertex : BigDNA
    {
        DECL_DNA
        Value<atVec3f> position;
        Value<atVec3f> normal;
    };
    Value<atUint32> vertexCount;
    Vector<Vertex, DNA_COUNT(vertexCount)> nodes;
    struct Edge : BigDNA
    {
        DECL_DNA
        Value<atUint32> polyA;
        Value<atUint32> polyB;
        Value<float> width1;
        Value<float> width2;
    };

    Value<atUint32> edgeCount;
    Vector<Edge, DNA_COUNT(edgeCount)> edges;

    struct Polygon : BigDNA
    {
        DECL_DNA
        Value<atUint32> vertCount;
        Value<atUint32> vertStart;
        Value<atUint32> edgeCount;
        Value<atUint32> edgeStart;
        Value<atUint32> flags;
        Value<float> area;
        Value<atVec3f> normal;
        Value<atUint32> selfIdx1;
        Value<atVec3f> center;
        Value<atVec3f> aabb[2];
        Value<atUint32> selfIdx2;
    };

    Value<atUint32> polyCount;
    Vector<Polygon, DNA_COUNT(polyCount)> polygons;

    Vector<atUint32, DNA_COUNT((((polyCount * (polyCount - 1)) / 2) + 31) / 32)> bitmap1;
    Vector<atUint32, DNA_COUNT(bitmap1.size())> bitmap2;
    Vector<atUint32, DNA_COUNT(((((polyCount * polyCount) + 31) / 32) - bitmap1.size()) * 2)> bitmap3;

    Value<atUint32> unkIntCount;
    Vector<atUint32, DNA_COUNT(unkIntCount)> unkInts;

    struct UnknownStruct : BigDNA
    {
        DECL_DNA
        Value<atUint32> unk1;
        Value<atVec3f> unk2[3];
        Value<atUint32> unk3[8]; // Usually 0xFF
        Value<atUint32> unk4;
        Value<atUint32> unk5;
    };

    Value<atUint32> unkStructCount;
    Vector<UnknownStruct, DNA_COUNT(unkStructCount)> unkStructs;
};
}

#endif // __DNACOMMON_PATH_HPP__
