#ifndef _DNAMP1_NEWCAMERASHAKER_HPP_
#define _DNAMP1_NEWCAMERASHAKER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct NewCameraShaker : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<bool> active;
    PlayerParameters flags;
    Value<float> duration;
    Value<float> sfxDist;
    struct CameraShakerComponent : BigYAML
    {
        DECL_YAML
        PlayerParameters flags;
        struct CameraShakePoint : BigYAML
        {
            DECL_YAML
            PlayerParameters flags;
            Value<float> attackTime;
            Value<float> sustainTime;
            Value<float> duration;
            Value<float> magnitude;
        };
        CameraShakePoint am;
        CameraShakePoint fm;
    } shakerComponents[3];
};
}

#endif
