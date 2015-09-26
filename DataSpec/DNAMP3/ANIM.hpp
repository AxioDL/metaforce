#ifndef _DNAMP3_ANIM_HPP_
#define _DNAMP3_ANIM_HPP_

#include "BlenderConnection.hpp"
#include "DNAMP3.hpp"
#include "../DNACommon/ANIM.hpp"
#include "CINF.hpp"

namespace Retro
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

        void sendANIMToBlender(HECL::BlenderConnection::PyOutStream&, const CINF&) const;
    };

    struct ANIM0 : IANIM
    {
        DECL_EXPLICIT_DNA
        ANIM0() : IANIM(0) {}

        struct Header : BigDNA
        {
            DECL_DNA
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
            Value<atUint32> unk6;
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
    void read(Athena::io::IStreamReader& reader)
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
            Log.report(LogVisor::FatalError, "unrecognized ANIM version");
            break;
        }
    }

    void write(Athena::io::IStreamWriter& writer) const
    {
        writer.writeUint32Big(m_anim->m_version);
        m_anim->write(writer);
    }

    void sendANIMToBlender(HECL::BlenderConnection::PyOutStream& os, const CINF& cinf) const
    {
        m_anim->sendANIMToBlender(os, cinf);
    }

};

}
}

#endif // _DNAMP3_ANIM_HPP_
