#ifndef _DNAMP3_ANIM_HPP_
#define _DNAMP3_ANIM_HPP_

#include "DNAMP3.hpp"
#include "DataSpec/DNACommon/ANIM.hpp"
#include "DataSpec/DNACommon/RigInverter.hpp"
#include "CINF.hpp"

namespace DataSpec::DNAMP3
{

struct ANIM : BigDNA
{
    AT_DECL_EXPLICIT_DNA

    struct IANIM : BigDNAV
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

        void sendANIMToBlender(hecl::blender::PyOutStream&,
                               const DNAANIM::RigInverter<CINF>& rig,
                               bool additive) const;
    };

    struct ANIM0 : IANIM
    {
        AT_DECL_EXPLICIT_DNA
        AT_DECL_DNAV
        ANIM0() : IANIM(0) {}

        struct Header : BigDNA
        {
            AT_DECL_DNA
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
        AT_DECL_EXPLICIT_DNA
        AT_DECL_DNAV
        ANIM1() : IANIM(1) {}

        struct Header : BigDNA
        {
            AT_DECL_DNA
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

    void sendANIMToBlender(hecl::blender::PyOutStream& os,
                           const DNAANIM::RigInverter<CINF>& rig,
                           bool additive) const
    {
        m_anim->sendANIMToBlender(os, rig, additive);
    }

};

}

#endif // _DNAMP3_ANIM_HPP_
