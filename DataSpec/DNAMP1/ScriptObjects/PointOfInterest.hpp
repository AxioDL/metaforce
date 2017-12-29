#ifndef _DNAMP1_POINTOFINTEREST_HPP_
#define _DNAMP1_POINTOFINTEREST_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct PointOfInterest : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    ScannableParameters scannableParameters;
    Value<float> unknown2;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        scannableParameters.nameIDs(pakRouter, name + "_scanp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        scannableParameters.depIDs(pathsOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        scannableParameters.scanIDs(scansOut);
    }
};
}

#endif
