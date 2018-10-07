#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Sound : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
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

