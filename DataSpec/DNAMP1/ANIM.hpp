#pragma once

#include "DNAMP1.hpp"
#include "DataSpec/DNACommon/ANIM.hpp"
#include "DataSpec/DNACommon/RigInverter.hpp"
#include "CINF.hpp"
#include "EVNT.hpp"
#include "DataSpec/DNACommon/ANCS.hpp"

namespace DataSpec::DNAMP1 {

struct ANIM : BigDNA {
  AT_DECL_EXPLICIT_DNA

  static UniqueID32 GetEVNTId(athena::io::IStreamReader& r);

  struct IANIM : BigDNAV {
    Delete expl;
    atUint32 m_version;
    IANIM(atUint32 version) : m_version(version) {}

    std::vector<std::pair<atUint32, bool>> bones;
    std::vector<atUint32> frames;
    std::vector<DNAANIM::Channel> channels;
    std::vector<std::vector<DNAANIM::Value>> chanKeys;
    float mainInterval = 0.0;
    UniqueID32Zero evnt;
    bool looping = false;

    void sendANIMToBlender(hecl::blender::PyOutStream&, const DNAANIM::RigInverter<CINF>& rig) const;
  };

  struct ANIM0 : IANIM {
    AT_DECL_EXPLICIT_DNAV
    ANIM0() : IANIM(0) {}

    struct Header : BigDNA {
      AT_DECL_DNA
      Value<float> duration;
      Value<atUint32> unk0;
      Value<float> interval;
      Value<atUint32> unk1;
      Value<atUint32> keyCount;
      Value<atUint32> unk2;
      Value<atUint32> boneSlotCount;
    };
  };

  struct ANIM2 : IANIM {
    AT_DECL_EXPLICIT_DNAV
    ANIM2(bool pc) : IANIM(pc ? 3 : 2) {}

    struct Header : BigDNA {
      AT_DECL_DNA
      Value<atUint32> scratchSize;
      UniqueID32Zero evnt;
      Value<atUint32> unk0 = 1;
      Value<float> duration;
      Value<float> interval;
      Value<atUint32> rootBoneId = 3;
      Value<atUint32> looping = 0;
      Value<atUint32> rotDiv;
      Value<float> translationMult;
      Value<atUint32> boneChannelCount;
      Value<atUint32> unk3;
      Value<atUint32> keyBitmapBitCount;
    };

    struct ChannelDesc : BigDNA {
      Delete expl;
      Value<atUint32> id = 0;
      Value<atUint16> keyCount1 = 0;
      Value<atInt16> initRX = 0;
      Value<atUint8> qRX = 0;
      Value<atInt16> initRY = 0;
      Value<atUint8> qRY = 0;
      Value<atInt16> initRZ = 0;
      Value<atUint8> qRZ = 0;
      Value<atUint16> keyCount2 = 0;
      Value<atInt16> initTX = 0;
      Value<atUint8> qTX = 0;
      Value<atInt16> initTY = 0;
      Value<atUint8> qTY = 0;
      Value<atInt16> initTZ = 0;
      Value<atUint8> qTZ = 0;

      void read(athena::io::IStreamReader& reader) {
        id = reader.readUint32Big();
        keyCount1 = reader.readUint16Big();
        initRX = reader.readInt16Big();
        qRX = reader.readUByte();
        initRY = reader.readInt16Big();
        qRY = reader.readUByte();
        initRZ = reader.readInt16Big();
        qRZ = reader.readUByte();
        keyCount2 = reader.readUint16Big();
        if (keyCount2) {
          initTX = reader.readInt16Big();
          qTX = reader.readUByte();
          initTY = reader.readInt16Big();
          qTY = reader.readUByte();
          initTZ = reader.readInt16Big();
          qTZ = reader.readUByte();
        }
      }
      void write(athena::io::IStreamWriter& writer) const {
        writer.writeUint32Big(id);
        writer.writeUint16Big(keyCount1);
        writer.writeInt16Big(initRX);
        writer.writeUByte(qRX);
        writer.writeInt16Big(initRY);
        writer.writeUByte(qRY);
        writer.writeInt16Big(initRZ);
        writer.writeUByte(qRZ);
        writer.writeUint16Big(keyCount2);
        if (keyCount2) {
          writer.writeInt16Big(initTX);
          writer.writeUByte(qTX);
          writer.writeInt16Big(initTY);
          writer.writeUByte(qTY);
          writer.writeInt16Big(initTZ);
          writer.writeUByte(qTZ);
        }
      }
      void binarySize(size_t& __isz) const {
        __isz += 17;
        if (keyCount2)
          __isz += 9;
      }
    };

    struct ChannelDescPC : BigDNA {
      Delete expl;
      Value<atUint32> id = 0;
      Value<atUint32> keyCount1 = 0;
      Value<atUint32> QinitRX = 0;
      Value<atUint32> QinitRY = 0;
      Value<atUint32> QinitRZ = 0;
      Value<atUint32> keyCount2 = 0;
      Value<atUint32> QinitTX = 0;
      Value<atUint32> QinitTY = 0;
      Value<atUint32> QinitTZ = 0;

      void read(athena::io::IStreamReader& reader) {
        id = reader.readUint32Big();
        keyCount1 = reader.readUint32Big();
        QinitRX = reader.readUint32Big();
        QinitRY = reader.readUint32Big();
        QinitRZ = reader.readUint32Big();
        keyCount2 = reader.readUint32Big();
        if (keyCount2) {
          QinitTX = reader.readUint32Big();
          QinitTY = reader.readUint32Big();
          QinitTZ = reader.readUint32Big();
        }
      }
      void write(athena::io::IStreamWriter& writer) const {
        writer.writeUint32Big(id);
        writer.writeUint32Big(keyCount1);
        writer.writeUint32Big(QinitRX);
        writer.writeUint32Big(QinitRY);
        writer.writeUint32Big(QinitRZ);
        writer.writeUint32Big(keyCount2);
        if (keyCount2) {
          writer.writeUint32Big(QinitTX);
          writer.writeUint32Big(QinitTY);
          writer.writeUint32Big(QinitTZ);
        }
      }
      void binarySize(size_t& __isz) const {
        __isz += 24;
        if (keyCount2)
          __isz += 12;
      }
    };
  };

  std::unique_ptr<IANIM> m_anim;

  void sendANIMToBlender(hecl::blender::PyOutStream& os, const DNAANIM::RigInverter<CINF>& rig, bool) const {
    m_anim->sendANIMToBlender(os, rig);
  }

  bool isLooping() const {
    if (!m_anim)
      return false;
    return m_anim->looping;
  }

  void extractEVNT(const DNAANCS::AnimationResInfo<UniqueID32>& animInfo, const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter, bool force) const {
    if (m_anim->evnt.isValid()) {
      hecl::SystemStringConv sysStr(animInfo.name);
      hecl::ProjectPath evntYamlPath = outPath.getWithExtension(
          fmt::format(fmt(_SYS_STR(".{}_{}.evnt.yaml")), sysStr, m_anim->evnt).c_str(), true);
      hecl::ProjectPath::Type evntYamlType = evntYamlPath.getPathType();

      if (force || evntYamlType == hecl::ProjectPath::Type::None) {
        EVNT evnt;
        if (pakRouter.lookupAndReadDNA(m_anim->evnt, evnt, true)) {
          athena::io::FileWriter writer(evntYamlPath.getAbsolutePath());
          athena::io::ToYAMLStream(evnt, writer);
        }
      }
    }
  }

  using BlenderAction = hecl::blender::Action;

  ANIM() = default;
  ANIM(const BlenderAction& act, const std::unordered_map<std::string, atInt32>& idMap,
       const DNAANIM::RigInverter<CINF>& rig, bool pc);
};

} // namespace DataSpec::DNAMP1
