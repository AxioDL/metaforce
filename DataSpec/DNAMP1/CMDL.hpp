#ifndef _DNAMP1_CMDL_HPP_
#define _DNAMP1_CMDL_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "CMDLMaterials.hpp"
#include "BlenderConnection.hpp"

namespace Retro
{
namespace DNAMP1
{

struct CMDL
{
    struct Header : BigDNA
    {
        DECL_DNA
        Value<atUint32> magic;
        Value<atUint32> version;
        struct Flags
        {
            DECL_DNA
            Value<atUint32> flags;
            inline bool shortNormals() const {return (flags & 0x2) != 0;}
            inline void setShortNormals(bool val) {flags &= ~0x2; flags |= val << 1;}
            inline bool shortUVs() const {return (flags & 0x4) != 0;}
            inline void setShortUVs(bool val) {flags &= ~0x4; flags |= val << 2;}
        } flags;
        Value<atVec3f> aabbMin;
        Value<atVec3f> aabbMax;
        Value<atUint32> secCount;
        Value<atUint32> matSetCount;
        Vector<atUint32, DNA_COUNT(secCount)> secSizes;
    };

    struct SurfaceHeader : BigDNA
    {
        DECL_DNA
        Value<atVec3f> centroid;
        Value<atUint32> matIdx;
        Value<atInt16> qDiv;
        Value<atUint16> dlSize;
        Seek<8, Athena::Current> seek;
        Value<atUint32> aabbSz;
        Value<atVec3f> reflectionNormal;
        Seek<DNA_COUNT(aabbSz), Athena::Current> seek2;
        Align<32> align;
    };

    static bool ReadToBlender(HECL::BlenderConnection& conn, Athena::io::IStreamReader& reader);

    static bool Extract(PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
    {
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        if (!conn.createBlend(outPath.getAbsolutePath()))
            return false;
        return ReadToBlender(conn, rs);
    }
};

}
}

#endif // _DNAMP1_CMDL_HPP_
