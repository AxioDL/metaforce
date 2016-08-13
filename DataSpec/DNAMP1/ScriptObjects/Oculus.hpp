#ifndef _DNAMP1_OCULUS_HPP_
#define _DNAMP1_OCULUS_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct Oculus : IScriptObject
{
    Delete expl;
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    DamageVulnerability damageVulnerabilty;
    Value<float> unknown7;
    DamageInfo damageInfo;

    /* Trilogy addition */
    Value<float> unknown8;

    void read(athena::io::IStreamReader& __dna_reader)
    {
        IScriptObject::read(__dna_reader);
        /* name */
        name = __dna_reader.readString(-1);
        /* location */
        location = __dna_reader.readVec3fBig();
        /* orientation */
        orientation = __dna_reader.readVec3fBig();
        /* scale */
        scale = __dna_reader.readVec3fBig();
        /* patternedInfo */
        patternedInfo.read(__dna_reader);
        /* actorParameters */
        actorParameters.read(__dna_reader);
        /* unknown1 */
        unknown1 = __dna_reader.readFloatBig();
        /* unknown2 */
        unknown2 = __dna_reader.readFloatBig();
        /* unknown3 */
        unknown3 = __dna_reader.readFloatBig();
        /* unknown4 */
        unknown4 = __dna_reader.readFloatBig();
        /* unknown5 */
        unknown5 = __dna_reader.readFloatBig();
        /* unknown6 */
        unknown6 = __dna_reader.readFloatBig();
        /* damageVulnerabilty */
        damageVulnerabilty.read(__dna_reader);
        /* unknown7 */
        unknown7 = __dna_reader.readFloatBig();
        /* damageInfo */
        damageInfo.read(__dna_reader);

        if (propertyCount == 16)
            unknown8 = __dna_reader.readFloatBig();
        else
            unknown8 = 0.0;
    }

    void write(athena::io::IStreamWriter& __dna_writer) const
    {
        IScriptObject::write(__dna_writer);
        /* name */
        __dna_writer.writeString(name, -1);
        /* location */
        __dna_writer.writeVec3fBig(location);
        /* orientation */
        __dna_writer.writeVec3fBig(orientation);
        /* scale */
        __dna_writer.writeVec3fBig(scale);
        /* patternedInfo */
        patternedInfo.write(__dna_writer);
        /* actorParameters */
        actorParameters.write(__dna_writer);
        /* unknown1 */
        __dna_writer.writeFloatBig(unknown1);
        /* unknown2 */
        __dna_writer.writeFloatBig(unknown2);
        /* unknown3 */
        __dna_writer.writeFloatBig(unknown3);
        /* unknown4 */
        __dna_writer.writeFloatBig(unknown4);
        /* unknown5 */
        __dna_writer.writeFloatBig(unknown5);
        /* unknown6 */
        __dna_writer.writeFloatBig(unknown6);
        /* damageVulnerabilty */
        damageVulnerabilty.write(__dna_writer);
        /* unknown7 */
        __dna_writer.writeFloatBig(unknown7);
        /* damageInfo */
        damageInfo.write(__dna_writer);

        if (propertyCount == 16)
            __dna_writer.writeFloatBig(unknown8);
    }

    size_t binarySize(size_t __isz) const
    {
        __isz = IScriptObject::binarySize(__isz);
        __isz += name.size() + 1;
        __isz = patternedInfo.binarySize(__isz);
        __isz = actorParameters.binarySize(__isz);
        __isz = damageVulnerabilty.binarySize(__isz);
        __isz = damageInfo.binarySize(__isz);
        if (propertyCount == 16)
            __isz += 4;
        return __isz + 64;
    }

    void read(athena::io::YAMLDocReader& __dna_docin)
    {
        IScriptObject::read(__dna_docin);
        /* name */
        name = __dna_docin.readString("name");
        /* location */
        location = __dna_docin.readVec3f("location");
        /* orientation */
        orientation = __dna_docin.readVec3f("orientation");
        /* scale */
        scale = __dna_docin.readVec3f("scale");
        /* patternedInfo */
        __dna_docin.enumerate("patternedInfo", patternedInfo);
        /* actorParameters */
        __dna_docin.enumerate("actorParameters", actorParameters);
        /* unknown1 */
        unknown1 = __dna_docin.readFloat("unknown1");
        /* unknown2 */
        unknown2 = __dna_docin.readFloat("unknown2");
        /* unknown3 */
        unknown3 = __dna_docin.readFloat("unknown3");
        /* unknown4 */
        unknown4 = __dna_docin.readFloat("unknown4");
        /* unknown5 */
        unknown5 = __dna_docin.readFloat("unknown5");
        /* unknown6 */
        unknown6 = __dna_docin.readFloat("unknown6");
        /* damageVulnerabilty */
        __dna_docin.enumerate("damageVulnerabilty", damageVulnerabilty);
        /* unknown7 */
        unknown7 = __dna_docin.readFloat("unknown7");
        /* damageInfo */
        __dna_docin.enumerate("damageInfo", damageInfo);

        if (propertyCount == 16)
            unknown8 = __dna_docin.readFloat("unknown8");
        else
            unknown8 = 0.0;
    }

    void write(athena::io::YAMLDocWriter& __dna_docout) const
    {
        IScriptObject::write(__dna_docout);
        /* name */
        __dna_docout.writeString("name", name);
        /* location */
        __dna_docout.writeVec3f("location", location);
        /* orientation */
        __dna_docout.writeVec3f("orientation", orientation);
        /* scale */
        __dna_docout.writeVec3f("scale", scale);
        /* patternedInfo */
        __dna_docout.enumerate("patternedInfo", patternedInfo);
        /* actorParameters */
        __dna_docout.enumerate("actorParameters", actorParameters);
        /* unknown1 */
        __dna_docout.writeFloat("unknown1", unknown1);
        /* unknown2 */
        __dna_docout.writeFloat("unknown2", unknown2);
        /* unknown3 */
        __dna_docout.writeFloat("unknown3", unknown3);
        /* unknown4 */
        __dna_docout.writeFloat("unknown4", unknown4);
        /* unknown5 */
        __dna_docout.writeFloat("unknown5", unknown5);
        /* unknown6 */
        __dna_docout.writeFloat("unknown6", unknown6);
        /* damageVulnerabilty */
        __dna_docout.enumerate("damageVulnerabilty", damageVulnerabilty);
        /* unknown7 */
        __dna_docout.writeFloat("unknown7", unknown7);
        /* damageInfo */
        __dna_docout.enumerate("damageInfo", damageInfo);

        if (propertyCount == 16)
            __dna_docout.writeFloat("unknown8", unknown8);
    }

    static const char* DNAType()
    {
        return "urde::DNAMP1::Oculus";
    }

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }
};
}
}

#endif
