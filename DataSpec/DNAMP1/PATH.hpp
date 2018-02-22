#ifndef __DNACOMMON_PATH_HPP__
#define __DNACOMMON_PATH_HPP__

#include "../DNACommon/DNACommon.hpp"

namespace DataSpec
{
struct PATH : BigDNA
{
    AT_DECL_DNA
    Value<atUint32> version;

    struct Node : BigDNA
    {
        AT_DECL_DNA
        Value<atVec3f> position;
        Value<atVec3f> normal;
    };
    Value<atUint32> nodeCount;
    Vector<Node, DNA_COUNT(nodeCount)> nodes;

    struct Link : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> nodeIdx;
        Value<atUint32> regionIdx;
        Value<float> width2d;
        Value<float> oneOverWidth2d;
    };
    Value<atUint32> linkCount;
    Vector<Link, DNA_COUNT(linkCount)> links;

    struct Region : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> nodeCount;
        Value<atUint32> nodeStart;
        Value<atUint32> linkCount;
        Value<atUint32> linkStart;
        Value<atUint32> flags;
        Value<float> height;
        Value<atVec3f> normal;
        Value<atUint32> regionIdx;
        Value<atVec3f> centroid;
        Value<atVec3f> aabb[2];
        Value<atUint32> regionIdxPtr;
    };
    Value<atUint32> regionCount;
    Vector<Region, DNA_COUNT(regionCount)> regions;

    Vector<atUint32, DNA_COUNT((((regionCount * (regionCount - 1)) / 2) + 31) / 32)> bitmap1;
    Vector<atUint32, DNA_COUNT(bitmap1.size())> bitmap2;
    Vector<atUint32, DNA_COUNT(((((regionCount * regionCount) + 31) / 32) - bitmap1.size()) * 2)> bitmap3;

    Value<atUint32> octreeRegionLookupCount;
    Vector<atUint32, DNA_COUNT(octreeRegionLookupCount)> octreeRegionLookup;

    struct OctreeNode : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> isLeaf;
        Value<atVec3f> points[3];
        Value<atUint32> children[8];
        Value<atUint32> regionCount;
        Value<atUint32> regionStart;
    };
    Value<atUint32> octreeNodeCount;
    Vector<OctreeNode, DNA_COUNT(octreeNodeCount)> octree;
};
}

#endif // __DNACOMMON_PATH_HPP__
