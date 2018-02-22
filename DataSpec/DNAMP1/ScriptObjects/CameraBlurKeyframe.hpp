#ifndef _DNAMP1_CAMERABLURKEYFRAME_HPP_
#define _DNAMP1_CAMERABLURKEYFRAME_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct CameraBlurKeyframe : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<bool> active;
    Value<atUint32> btype;
    Value<float> amount;
    Value<atUint32> unk;
    Value<float> timeIn;
    Value<float> timeOut;
};
}

#endif
