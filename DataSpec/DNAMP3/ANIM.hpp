#ifndef _DNAMP3_ANIM_HPP_
#define _DNAMP3_ANIM_HPP_

#include "BlenderConnection.hpp"
#include "DNAMP3.hpp"
#include "../DNACommon/ANIM.hpp"
#include "../DNACommon/RigInverter.hpp"
#include "CINF.hpp"

namespace DataSpec
{
namespace DNAMP3
{

struct ANIM : BigDNA
{
    Delete expl;

    struct IANIM : BigDNA
    {
        Delete expl;
        atUint32 m_version;
        IANIM(atUint32 version) : m_version(version) {}

        std::vector<std::pair<atUint32, std::tuple<bool,bool,bool>>> bones;
        std::vector<atUint32> frames;
        std::vector<DNAANIM::Channel> channels;
        std::vector<std::vector<DNAANIM::Value>> chanKeys;
        float mainInterval = 0.0;
        bool looping = false;

        void sendANIMToBlender(hecl::BlenderConnection::PyOutStream&,
                               const DNAANIM::RigInverter<CINF>& rig,
                               bool additive) const;
    };

    struct ANIM0 : IANIM
    {
        DECL_EXPLICIT_DNA
        ANIM0() : IANIM(0) {}

        struct Header : BigDNA
        {
            DECL_DNA
            Value<atUint16> unkS;
            Value<float> duration;
            Value<atUint32> unk0;
            Value<float> interval;
            Value<atUint32> unk1;
            Value<atUint32> keyCount;
            Value<atUint32> unk2;
            Value<atUint32> boneSlotCount;
        };
    };

    struct ANIM1 : IANIM
    {
        DECL_EXPLICIT_DNA
        ANIM1() : IANIM(1) {}

        struct Header : BigDNA
        {
            DECL_DNA
            Value<atUint16> unk1;
            Value<atUint8> unk2;
            Value<atUint32> unk3;
            Value<atUint8> unk4[3];
            Value<float> translationMult;
            Value<float> scaleMult;
            Value<atUint32> unk7;
            Value<float> unk8;
            Value<float> unk9;
            Value<float> duration;
            Value<atUint16> keyCount;
            Value<atUint32> blobSize;
            Value<atUint8> unk10;
            Value<atUint32> floatsSize;
            Value<atUint32> flagsSize;
            Value<atUint32> initBlockSize;
            Value<atUint32> streamSize;
            Value<atUint32> unk11;
            Value<atUint32> boneCount;
        };
    };

    std::unique_ptr<IANIM> m_anim;
    void read(athena::io::IStreamReader& reader)
    {
        atUint32 version = reader.readUint32Big();
        switch (version)
        {
        case 0:
            m_anim.reset(new struct ANIM0);
            m_anim->read(reader);
            break;
        case 1:
            m_anim.reset(new struct ANIM1);
            m_anim->read(reader);
            break;
        default:
            Log.report(logvisor::Fatal, "unrecognized ANIM version");
            break;
        }
    }

    void write(athena::io::IStreamWriter& writer) const
    {
        writer.writeUint32Big(m_anim->m_version);
        m_anim->write(writer);
    }

    size_t binarySize(size_t __isz) const
    {
        return m_anim->binarySize(__isz + 4);
    }

    void sendANIMToBlender(hecl::BlenderConnection::PyOutStream& os,
                           const DNAANIM::RigInverter<CINF>& rig,
                           bool additive) const
    {
        m_anim->sendANIMToBlender(os, rig, additive);
    }

};

}
}

#endif // _DNAMP3_ANIM_HPP_
