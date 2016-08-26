#ifndef _DNAMP1_ANIM_HPP_
#define _DNAMP1_ANIM_HPP_

#include "BlenderConnection.hpp"
#include "DNAMP1.hpp"
#include "../DNACommon/ANIM.hpp"
#include "DataSpec/DNACommon/RigInverter.hpp"
#include "CINF.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct ANIM : BigDNA
{
    Delete expl;

    struct IANIM : BigDNA
    {
        Delete expl;
        atUint32 m_version;
        IANIM(atUint32 version) : m_version(version) {}

        std::vector<std::pair<atUint32, bool>> bones;
        std::vector<atUint32> frames;
        std::vector<DNAANIM::Channel> channels;
        std::vector<std::vector<DNAANIM::Value>> chanKeys;
        float mainInterval = 0.0;
        UniqueID32 evnt;

        void sendANIMToBlender(hecl::BlenderConnection::PyOutStream&, const DNAANIM::RigInverter<CINF>& rig) const;
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

    struct ANIM2 : IANIM
    {
        DECL_EXPLICIT_DNA
        ANIM2(bool pc) : IANIM(pc ? 3 : 2) {}

        struct Header : BigDNA
        {
            DECL_DNA
            Value<atUint32> scratchSize;
            UniqueID32 evnt;
            Value<atUint32> unk0 = 1;
            Value<float> duration;
            Value<float> interval;
            Value<atUint32> rootBoneId = 3;
            Value<atUint32> unk2 = 0;
            Value<atUint32> rotDiv;
            Value<float> translationMult;
            Value<atUint32> boneChannelCount;
            Value<atUint32> unk3;
            Value<atUint32> keyBitmapBitCount;
        };

        struct ChannelDesc : BigDNA
        {
            Delete expl;
            Value<atUint32> id = 0;
            Value<atUint16> keyCount1 = 0;
            Value<atUint16> initRX = 0;
            Value<atUint8> qRX = 0;
            Value<atUint16> initRY = 0;
            Value<atUint8> qRY = 0;
            Value<atUint16> initRZ = 0;
            Value<atUint8> qRZ = 0;
            Value<atUint16> keyCount2 = 0;
            Value<atUint16> initTX = 0;
            Value<atUint8> qTX = 0;
            Value<atUint16> initTY = 0;
            Value<atUint8> qTY = 0;
            Value<atUint16> initTZ = 0;
            Value<atUint8> qTZ = 0;

            void read(athena::io::IStreamReader& reader)
            {
                id = reader.readUint32Big();
                keyCount1 = reader.readUint16Big();
                initRX = reader.readUint16Big();
                qRX = reader.readUByte();
                initRY = reader.readUint16Big();
                qRY = reader.readUByte();
                initRZ = reader.readUint16Big();
                qRZ = reader.readUByte();
                keyCount2 = reader.readUint16Big();
                if (keyCount2)
                {
                    initTX = reader.readUint16Big();
                    qTX = reader.readUByte();
                    initTY = reader.readUint16Big();
                    qTY = reader.readUByte();
                    initTZ = reader.readUint16Big();
                    qTZ = reader.readUByte();
                }
            }
            void write(athena::io::IStreamWriter& writer) const
            {
                writer.writeUint32Big(id);
                writer.writeUint16Big(keyCount1);
                writer.writeUint16Big(initRX);
                writer.writeUByte(qRX);
                writer.writeUint16Big(initRY);
                writer.writeUByte(qRY);
                writer.writeUint16Big(initRZ);
                writer.writeUByte(qRZ);
                writer.writeUint16Big(keyCount2);
                if (keyCount2)
                {
                    writer.writeUint16Big(initTX);
                    writer.writeUByte(qTX);
                    writer.writeUint16Big(initTY);
                    writer.writeUByte(qTY);
                    writer.writeUint16Big(initTZ);
                    writer.writeUByte(qTZ);
                }
            }
            size_t binarySize(size_t __isz) const
            {
                __isz += 17;
                if (keyCount2)
                    __isz += 9;
                return __isz;
            }
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
        case 2:
            m_anim.reset(new struct ANIM2(false));
            m_anim->read(reader);
            break;
        case 3:
            m_anim.reset(new struct ANIM2(true));
            m_anim->read(reader);
            break;
        default:
            Log.report(logvisor::Error, "unrecognized ANIM version");
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

    void sendANIMToBlender(hecl::BlenderConnection::PyOutStream& os, const DNAANIM::RigInverter<CINF>& rig, bool) const
    {
        m_anim->sendANIMToBlender(os, rig);
    }

    using BlenderAction = hecl::BlenderConnection::DataStream::Actor::Action;

    ANIM() = default;
    ANIM(const BlenderAction& act,
         const std::unordered_map<std::string, atInt32>& idMap,
         const DNAANIM::RigInverter<CINF>& rig,
         bool pc);
};

}
}

#endif // _DNAMP1_ANIM_HPP_
