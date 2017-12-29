#ifndef _DNAMP1_SOUND_HPP_
#define _DNAMP1_SOUND_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Sound : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atUint32> soundID;
    Value<bool> active;
    Value<float> maxDist;
    Value<float> distComp;
    Value<float> startDelay;
    Value<atUint32> minVol;
    Value<atUint32> vol;
    Value<atUint32> prio;
    Value<atUint32> pan;
    Value<bool> loop;
    Value<bool> nonEmitter;
    Value<bool> autoStart;
    Value<bool> occlusionTest;
    Value<bool> acoustics;
    Value<bool> worldSfx;
    Value<bool> allowDuplicates;
    Value<atUint32> pitch;
};
}

#endif
