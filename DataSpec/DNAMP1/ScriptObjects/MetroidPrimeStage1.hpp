#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct MetroidPrimeStage1 : IScriptObject {
  AT_DECL_DNA_YAMLV
  Value<atUint32> version;
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  Value<bool> unknown2;
  Value<float> unknown3;
  Value<float> unknown4;
  Value<float> unknown5;
  Value<atUint32> unknown6;
  Value<bool> unknown7;
  Value<atUint32> unknown8;
  HealthInfo healthInfo1;
  HealthInfo healthInfo2;
  Value<atUint32> unknown9;

  struct PrimeParameters1 : BigDNA {
    AT_DECL_DNA
    Value<atUint32> propertyCount;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<float> unknown10;
    Value<float> unknown11;
    Value<float> unknown12;
    Value<float> unknown13;
    Value<float> unknown14;
  } primeStruct1[4];

  Value<atUint32> unknown10;
  Value<atUint32> unknown11;

  struct MassivePrimeStruct : BigDNA {
    AT_DECL_DNA
    Value<atUint32> propertyCount;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<atUint32> unknown2;
    struct CameraShakeData : BigDNA {
      AT_DECL_DNA
      Value<bool> useSfx;
      Value<float> duration;
      Value<float> sfxDist;
      struct CameraShakerComponent : BigDNA {
        AT_DECL_DNA
        Value<bool> useModulation;
        struct CameraShakePoint : BigDNA {
          AT_DECL_DNA
          Value<float> attackTime;
          Value<float> sustainTime;
          Value<float> duration;
          Value<float> magnitude;
        };
        CameraShakePoint am;
        CameraShakePoint fm;
      } shakerComponents[3];
    } shakeDatas[3];

    struct PrimeStruct2B : BigDNA {
      AT_DECL_DNA
      Value<atUint32> propertyCount;
      UniqueID32 particle1;
      UniqueID32 particle2;
      UniqueID32 particle3;
      DamageInfo damageInfo1;
      Value<float> unknown5;
      Value<float> unknown6;
      UniqueID32 texture1;
      Value<atUint32> unknown7;
      Value<atUint32> unknown8;

      void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
        if (particle1.isValid()) {
          PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
          ent->name = name + "_part1";
        }
        if (particle2.isValid()) {
          PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle2);
          ent->name = name + "_part2";
        }
        if (particle3.isValid()) {
          PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle3);
          ent->name = name + "_part3";
        }
        if (texture1.isValid()) {
          PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture1);
          ent->name = name + "_tex1";
        }
      }

      void depIDs(std::vector<hecl::ProjectPath>& pathsOut) const {
        g_curSpec->flattenDependencies(particle1, pathsOut);
        g_curSpec->flattenDependencies(particle2, pathsOut);
        g_curSpec->flattenDependencies(particle3, pathsOut);
        g_curSpec->flattenDependencies(texture1, pathsOut);
      }
    } primeStruct2b;

    UniqueID32 particle4;

    struct PrimeStruct4 : BigDNA {
      AT_DECL_DNA
      BeamInfo beamInfo;
      UniqueID32 wpsc;
      DamageInfo damageInfo1;
      struct PrimeStruct5 : BigDNA {
        AT_DECL_DNA
        Value<atUint32> propertyCount;
        UniqueID32 unknown1;
        Value<atUint32> unknown2;
        UniqueID32 unknown3;
        UniqueID32 unknown4;
        Value<atUint32> unknown5;
        Value<atUint32> unknown6;
        Value<atUint32> unknown7;
        Value<atUint32> unknown8;

        void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
          if (unknown1.isValid()) {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(unknown1);
            ent->name = name + "_unk1";
          }
          if (unknown3.isValid()) {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(unknown3);
            ent->name = name + "_unk3";
          }
          if (unknown4.isValid()) {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(unknown4);
            ent->name = name + "_unk4";
          }
        }

        void depIDs(std::vector<hecl::ProjectPath>& pathsOut) const {
          g_curSpec->flattenDependencies(unknown1, pathsOut);
          g_curSpec->flattenDependencies(unknown3, pathsOut);
          g_curSpec->flattenDependencies(unknown4, pathsOut);
        }
      } primeStruct5;
      Value<float> unknown14;
      DamageInfo damageInfo2;

      void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
        beamInfo.nameIDs(pakRouter, name + "_beamInfo");
        if (wpsc.isValid()) {
          PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc);
          ent->name = name + "_wpsc";
        }
        primeStruct5.nameIDs(pakRouter, name + "_prime5");
      }

      void depIDs(std::vector<hecl::ProjectPath>& pathsOut) const {
        beamInfo.depIDs(pathsOut);
        g_curSpec->flattenDependencies(wpsc, pathsOut);
        primeStruct5.depIDs(pathsOut);
      }
    } primeStruct4s[4];

    UniqueID32 wpsc1;
    DamageInfo damageInfo2;
    CameraShakeData primeStruct2_4;
    UniqueID32 wpsc2;
    DamageInfo damageInfo3;
    CameraShakeData primeStruct2_5;

    struct PrimeProjectileInfo : BigDNA {
      AT_DECL_DNA
      Value<atUint32> propertyCount;
      UniqueID32 particle;
      DamageInfo damageInfo4;
      Value<float> unknown9;
      Value<float> unknown10;
      Value<float> unknown11;
      UniqueID32 texture;
      Value<bool> unknown12;
      Value<bool> unknown13;
      Value<bool> unknown14;
      Value<bool> unknown15;

      void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
        if (particle.isValid()) {
          PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
          ent->name = name + "_part";
        }
        if (texture.isValid()) {
          PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture);
          ent->name = name + "_tex";
        }
      }

      void depIDs(std::vector<hecl::ProjectPath>& pathsOut) const {
        g_curSpec->flattenDependencies(particle, pathsOut);
        g_curSpec->flattenDependencies(texture, pathsOut);
      }
    } projectileInfo;

    DamageInfo damageInfo5;
    CameraShakeData primeStruct2_6;
    UniqueID32 particle6;
    UniqueID32 swhc;
    UniqueID32 particle7;
    UniqueID32 particle8;

    struct PrimeStruct6 : BigDNA {
      AT_DECL_DNA
      Value<atUint32> propertyCount;
      DamageVulnerability damageVulnerability;
      DNAColor unknown1;
      Value<atUint32> unknown2;
      Value<atUint32> unknown3;
    } primeStruct6s[4];

    void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
      primeStruct2b.nameIDs(pakRouter, name + "_prime2b");
      if (particle4.isValid()) {
        PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle4);
        ent->name = name + "_part4";
      }
      if (particle6.isValid()) {
        PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle6);
        ent->name = name + "_part6";
      }
      if (particle7.isValid()) {
        PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle7);
        ent->name = name + "_part7";
      }
      if (particle8.isValid()) {
        PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle8);
        ent->name = name + "_part8";
      }
      if (swhc.isValid()) {
        PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(swhc);
        ent->name = name + "_swhc";
      }
      if (wpsc1.isValid()) {
        PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc1);
        ent->name = name + "_wpsc1";
      }
      if (wpsc2.isValid()) {
        PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc2);
        ent->name = name + "_wpsc2";
      }
      patternedInfo.nameIDs(pakRouter, name + "_patterned");
      actorParameters.nameIDs(pakRouter, name + "_actp");
      primeStruct4s[0].nameIDs(pakRouter, name + "_prime41");
      primeStruct4s[1].nameIDs(pakRouter, name + "_prime42");
      primeStruct4s[2].nameIDs(pakRouter, name + "_prime43");
      primeStruct4s[3].nameIDs(pakRouter, name + "_prime44");
      projectileInfo.nameIDs(pakRouter, name + "_projectileInfo");
    }

    void depIDs(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
      primeStruct2b.depIDs(pathsOut);
      g_curSpec->flattenDependencies(particle4, pathsOut);
      g_curSpec->flattenDependencies(particle6, pathsOut);
      g_curSpec->flattenDependencies(particle7, pathsOut);
      g_curSpec->flattenDependencies(particle8, pathsOut);
      g_curSpec->flattenDependencies(swhc, pathsOut);
      g_curSpec->flattenDependencies(wpsc1, pathsOut);
      g_curSpec->flattenDependencies(wpsc2, pathsOut);
      patternedInfo.depIDs(pathsOut);
      actorParameters.depIDs(pathsOut, lazyOut);
      primeStruct4s[0].depIDs(pathsOut);
      primeStruct4s[1].depIDs(pathsOut);
      primeStruct4s[2].depIDs(pathsOut);
      primeStruct4s[3].depIDs(pathsOut);
      projectileInfo.depIDs(pathsOut);
    }

    void scanIDs(std::vector<Scan>& scansOut) const { actorParameters.scanIDs(scansOut); }
  } massivePrimeStruct;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const override {
    massivePrimeStruct.actorParameters.addCMDLRigPairs(pakRouter, charAssoc,
                                                       massivePrimeStruct.patternedInfo.animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    massivePrimeStruct.nameIDs(pakRouter, name + "_massiveStruct");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    massivePrimeStruct.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { massivePrimeStruct.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
