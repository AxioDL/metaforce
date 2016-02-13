#ifndef __DNAMP1_EVNT_HPP__
#define __DNAMP1_EVNT_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct EVNT : BigYAML
{
    DECL_YAML
    Delete expl;
    Value<atUint32> version;

    struct EventBase : BigYAML
    {
        DECL_YAML
        Value<atUint16> unk0;
        String<-1> name;
        Value<atUint16> type;
        Value<float> startTime;
        Value<atUint32> unk1;
        Value<atUint32> idx;
        Value<atUint8> unk2;
        Value<float> unk3;
        Value<atUint32> unk4;
        Value<atUint32> unk5;
    };

    struct LoopEvent : EventBase
    {
        DECL_YAML
        Value<atUint8> flag;
    };
    std::vector<LoopEvent> loopEvents;

    struct UEVTEvent : EventBase
    {
        DECL_YAML
        Value<atUint32> uevtType;
        String<-1> boneName;
    };
    std::vector<UEVTEvent> uevtEvents;

    struct EffectEvent : EventBase
    {
        DECL_YAML
        Value<atUint32> frameCount;
        DNAFourCC effectType;
        UniqueID32 effectId;
        String<-1> boneName;
        Value<float> scale;
        Value<atUint32> parentMode;
    };
    std::vector<EffectEvent> effectEvents;

    struct SFXEvent : EventBase
    {
        DECL_YAML
        Value<atUint32> soundId;
        Value<float> smallNum;
        Value<float> bigNum;
    };
    std::vector<SFXEvent> sfxEvents;

    static bool Extract(PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
    {
        EVNT evnt;
        evnt.read(rs);
        FILE* fp = HECL::Fopen(outPath.getAbsolutePath().c_str(), _S("wb"));
        evnt.toYAMLFile(fp);
        fclose(fp);
        return true;
    }
};

}
}

#endif // __DNAMP1_EVNT_HPP__
