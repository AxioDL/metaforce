#ifndef _DNAMP1_RIDLEY_HPP_
#define _DNAMP1_RIDLEY_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Ridley : IScriptObject
{
    Delete expl;
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 model1;
    UniqueID32 model2;
    UniqueID32 model3;
    UniqueID32 model4;
    UniqueID32 model5;
    UniqueID32 model6;
    UniqueID32 model7;
    UniqueID32 model8;
    UniqueID32 model9;
    UniqueID32 model10;
    UniqueID32 model11;
    UniqueID32 model12;
    UniqueID32 particle;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    UniqueID32 wpsc1;
    DamageInfo damageInfo1;
    struct RidleyStruct1 : BigYAML
    {
        DECL_YAML
        Value<atUint32> unknown1;
        Value<atUint32> unknown2;
        UniqueID32 particle1;
        UniqueID32 particle2;
        UniqueID32 texture1;
        UniqueID32 texture2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
        Value<float> unknown9;
        Value<float> unknown10;
        Value<float> unknown11;
        Value<atVec4f> unknown12; //CColor
        Value<atVec4f> unknown13; //CColor

        void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const
        {
            if (particle1)
            {
                PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
                ent->name = name + "_part1";
            }
            if (particle2)
            {
                PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle2);
                ent->name = name + "_part2";
            }
            if (texture1)
            {
                PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture1);
                ent->name = name + "_tex1";
            }
            if (texture2)
            {
                PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture2);
                ent->name = name + "_tex2";
            }
        }
    } ridleyStruct1;

    Value<atUint32> soundID1;
    UniqueID32 wpsc2;
    UniqueID32 wpsc3;
    DamageInfo damageInfo2;

    struct RidleyStruct2 : BigYAML
    {
        DECL_YAML
        Value<atUint32> unknown1;
        Value<float> unknown2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
        Value<bool> unknown9;
    } ridleyStruct2_1;

    UniqueID32 wpsc4;
    DamageInfo damageInfo3;
    RidleyStruct2 ridleyStruct2_2;
    Value<atUint32> soundID2;
    DamageInfo damageInfo4;
    RidleyStruct2 ridleyStruct2_3;
    Value<float> unknown5;
    Value<float> unknown6;
    DamageInfo damageInfo5;
    Value<float> unknown7;
    DamageInfo damageInfo6;
    Value<float> unknown8;
    DamageInfo damageInfo7;
    Value<float> unknown9;
    UniqueID32 elsc;
    Value<float> unknown10;
    Value<atUint32> soundID3;
    DamageInfo damageInfo8;

    /* Trilogy addition */
    DamageInfo damageInfo9;

    void read(Athena::io::IStreamReader& __dna_reader)
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
        /* model1 */
        model1.read(__dna_reader);
        /* model2 */
        model2.read(__dna_reader);
        if (propertyCount == 48)
        {
            /* model3 */
            model3.read(__dna_reader);
            /* model4 */
            model4.read(__dna_reader);
            /* model5 */
            model5.read(__dna_reader);
            /* model6 */
            model6.read(__dna_reader);
            /* model7 */
            model7.read(__dna_reader);
            /* model8 */
            model8.read(__dna_reader);
            /* model9 */
            model9.read(__dna_reader);
            /* model10 */
            model10.read(__dna_reader);
            /* model11 */
            model11.read(__dna_reader);
            /* model12 */
            model12.read(__dna_reader);
        }
        /* particle */
        particle.read(__dna_reader);
        /* unknown1 */
        unknown1 = __dna_reader.readFloatBig();
        /* unknown2 */
        unknown2 = __dna_reader.readFloatBig();
        /* unknown3 */
        unknown3 = __dna_reader.readFloatBig();
        /* unknown4 */
        unknown4 = __dna_reader.readFloatBig();
        /* wpsc1 */
        wpsc1.read(__dna_reader);
        /* damageInfo1 */
        damageInfo1.read(__dna_reader);
        /* ridleyStruct1 */
        ridleyStruct1.read(__dna_reader);
        /* soundID1 */
        soundID1 = __dna_reader.readUint32Big();
        /* wpsc2 */
        wpsc2.read(__dna_reader);
        /* wpsc3 */
        if (propertyCount == 40)
            wpsc3.read(__dna_reader);
        /* damageInfo2 */
        damageInfo2.read(__dna_reader);
        /* ridleyStruct2_1 */
        ridleyStruct2_1.read(__dna_reader);
        /* wpsc4 */
        wpsc4.read(__dna_reader);
        /* damageInfo3 */
        damageInfo3.read(__dna_reader);
        /* ridleyStruct2_2 */
        ridleyStruct2_2.read(__dna_reader);
        /* soundID2 */
        soundID2 = __dna_reader.readUint32Big();
        /* damageInfo4 */
        damageInfo4.read(__dna_reader);
        /* ridleyStruct2_3 */
        ridleyStruct2_3.read(__dna_reader);
        /* unknown5 */
        unknown5 = __dna_reader.readFloatBig();
        /* unknown6 */
        unknown6 = __dna_reader.readFloatBig();
        /* damageInfo5 */
        damageInfo5.read(__dna_reader);
        /* unknown7 */
        unknown7 = __dna_reader.readFloatBig();
        /* damageInfo6 */
        damageInfo6.read(__dna_reader);
        /* unknown8 */
        unknown8 = __dna_reader.readFloatBig();
        /* damageInfo7 */
        damageInfo7.read(__dna_reader);
        /* unknown9 */
        unknown9 = __dna_reader.readFloatBig();
        /* elsc */
        elsc.read(__dna_reader);
        /* unknown10 */
        unknown10 = __dna_reader.readFloatBig();
        /* soundID3 */
        soundID3 = __dna_reader.readUint32Big();
        /* damageInfo8 */
        damageInfo8.read(__dna_reader);

        if (propertyCount == 40)
            damageInfo9.read(__dna_reader);
    }

    void write(Athena::io::IStreamWriter& __dna_writer) const
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
        /* model1 */
        model1.write(__dna_writer);
        /* model2 */
        model2.write(__dna_writer);
        if (propertyCount == 48)
        {
            /* model3 */
            model3.write(__dna_writer);
            /* model4 */
            model4.write(__dna_writer);
            /* model5 */
            model5.write(__dna_writer);
            /* model6 */
            model6.write(__dna_writer);
            /* model7 */
            model7.write(__dna_writer);
            /* model8 */
            model8.write(__dna_writer);
            /* model9 */
            model9.write(__dna_writer);
            /* model10 */
            model10.write(__dna_writer);
            /* model11 */
            model11.write(__dna_writer);
            /* model12 */
            model12.write(__dna_writer);
        }
        /* particle */
        particle.write(__dna_writer);
        /* unknown1 */
        __dna_writer.writeFloatBig(unknown1);
        /* unknown2 */
        __dna_writer.writeFloatBig(unknown2);
        /* unknown3 */
        __dna_writer.writeFloatBig(unknown3);
        /* unknown4 */
        __dna_writer.writeFloatBig(unknown4);
        /* wpsc1 */
        wpsc1.write(__dna_writer);
        /* damageInfo1 */
        damageInfo1.write(__dna_writer);
        /* ridleyStruct1 */
        ridleyStruct1.write(__dna_writer);
        /* soundID1 */
        __dna_writer.writeUint32Big(soundID1);
        /* wpsc2 */
        wpsc2.write(__dna_writer);
        /* wpsc3 */
        if (propertyCount == 40)
            wpsc3.write(__dna_writer);
        /* damageInfo2 */
        damageInfo2.write(__dna_writer);
        /* ridleyStruct2_1 */
        ridleyStruct2_1.write(__dna_writer);
        /* wpsc4 */
        wpsc4.write(__dna_writer);
        /* damageInfo3 */
        damageInfo3.write(__dna_writer);
        /* ridleyStruct2_2 */
        ridleyStruct2_2.write(__dna_writer);
        /* soundID2 */
        __dna_writer.writeUint32Big(soundID2);
        /* damageInfo4 */
        damageInfo4.write(__dna_writer);
        /* ridleyStruct2_3 */
        ridleyStruct2_3.write(__dna_writer);
        /* unknown5 */
        __dna_writer.writeFloatBig(unknown5);
        /* unknown6 */
        __dna_writer.writeFloatBig(unknown6);
        /* damageInfo5 */
        damageInfo5.write(__dna_writer);
        /* unknown7 */
        __dna_writer.writeFloatBig(unknown7);
        /* damageInfo6 */
        damageInfo6.write(__dna_writer);
        /* unknown8 */
        __dna_writer.writeFloatBig(unknown8);
        /* damageInfo7 */
        damageInfo7.write(__dna_writer);
        /* unknown9 */
        __dna_writer.writeFloatBig(unknown9);
        /* elsc */
        elsc.write(__dna_writer);
        /* unknown10 */
        __dna_writer.writeFloatBig(unknown10);
        /* soundID3 */
        __dna_writer.writeUint32Big(soundID3);
        /* damageInfo8 */
        damageInfo8.write(__dna_writer);

        if (propertyCount == 40)
            damageInfo9.write(__dna_writer);
    }

    void read(Athena::io::YAMLDocReader& __dna_docin)
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
        /* model1 */
        __dna_docin.enumerate("model1", model1);
        /* model2 */
        __dna_docin.enumerate("model2", model2);
        if (propertyCount == 48)
        {
            /* model3 */
            __dna_docin.enumerate("model3", model3);
            /* model4 */
            __dna_docin.enumerate("model4", model4);
            /* model5 */
            __dna_docin.enumerate("model5", model5);
            /* model6 */
            __dna_docin.enumerate("model6", model6);
            /* model7 */
            __dna_docin.enumerate("model7", model7);
            /* model8 */
            __dna_docin.enumerate("model8", model8);
            /* model9 */
            __dna_docin.enumerate("model9", model9);
            /* model10 */
            __dna_docin.enumerate("model10", model10);
            /* model11 */
            __dna_docin.enumerate("model11", model11);
            /* model12 */
            __dna_docin.enumerate("model12", model12);
        }
        /* particle */
        __dna_docin.enumerate("particle", particle);
        /* unknown1 */
        unknown1 = __dna_docin.readFloat("unknown1");
        /* unknown2 */
        unknown2 = __dna_docin.readFloat("unknown2");
        /* unknown3 */
        unknown3 = __dna_docin.readFloat("unknown3");
        /* unknown4 */
        unknown4 = __dna_docin.readFloat("unknown4");
        /* wpsc1 */
        __dna_docin.enumerate("wpsc1", wpsc1);
        /* damageInfo1 */
        __dna_docin.enumerate("damageInfo1", damageInfo1);
        /* ridleyStruct1 */
        __dna_docin.enumerate("ridleyStruct1", ridleyStruct1);
        /* soundID1 */
        soundID1 = __dna_docin.readUint32("soundID1");
        /* wpsc2 */
        __dna_docin.enumerate("wpsc2", wpsc2);
        /* wpsc3 */
        if (propertyCount == 40)
            __dna_docin.enumerate("wpsc3", wpsc3);
        /* damageInfo2 */
        __dna_docin.enumerate("damageInfo2", damageInfo2);
        /* ridleyStruct2_1 */
        __dna_docin.enumerate("ridleyStruct2_1", ridleyStruct2_1);
        /* wpsc4 */
        __dna_docin.enumerate("wpsc4", wpsc4);
        /* damageInfo3 */
        __dna_docin.enumerate("damageInfo3", damageInfo3);
        /* ridleyStruct2_2 */
        __dna_docin.enumerate("ridleyStruct2_2", ridleyStruct2_2);
        /* soundID2 */
        soundID2 = __dna_docin.readUint32("soundID2");
        /* damageInfo4 */
        __dna_docin.enumerate("damageInfo4", damageInfo4);
        /* ridleyStruct2_3 */
        __dna_docin.enumerate("ridleyStruct2_3", ridleyStruct2_3);
        /* unknown5 */
        unknown5 = __dna_docin.readFloat("unknown5");
        /* unknown6 */
        unknown6 = __dna_docin.readFloat("unknown6");
        /* damageInfo5 */
        __dna_docin.enumerate("damageInfo5", damageInfo5);
        /* unknown7 */
        unknown7 = __dna_docin.readFloat("unknown7");
        /* damageInfo6 */
        __dna_docin.enumerate("damageInfo6", damageInfo6);
        /* unknown8 */
        unknown8 = __dna_docin.readFloat("unknown8");
        /* damageInfo7 */
        __dna_docin.enumerate("damageInfo7", damageInfo7);
        /* unknown9 */
        unknown9 = __dna_docin.readFloat("unknown9");
        /* elsc */
        __dna_docin.enumerate("elsc", elsc);
        /* unknown10 */
        unknown10 = __dna_docin.readFloat("unknown10");
        /* soundID3 */
        soundID3 = __dna_docin.readUint32("soundID3");
        /* damageInfo8 */
        __dna_docin.enumerate("damageInfo8", damageInfo8);

        if (propertyCount == 40)
            __dna_docin.enumerate("damageInfo9", damageInfo9);
    }

    void write(Athena::io::YAMLDocWriter& __dna_docout) const
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
        /* model1 */
        __dna_docout.enumerate("model1", model1);
        /* model2 */
        __dna_docout.enumerate("model2", model2);
        if (propertyCount == 48)
        {
            /* model3 */
            __dna_docout.enumerate("model3", model3);
            /* model4 */
            __dna_docout.enumerate("model4", model4);
            /* model5 */
            __dna_docout.enumerate("model5", model5);
            /* model6 */
            __dna_docout.enumerate("model6", model6);
            /* model7 */
            __dna_docout.enumerate("model7", model7);
            /* model8 */
            __dna_docout.enumerate("model8", model8);
            /* model9 */
            __dna_docout.enumerate("model9", model9);
            /* model10 */
            __dna_docout.enumerate("model10", model10);
            /* model11 */
            __dna_docout.enumerate("model11", model11);
            /* model12 */
            __dna_docout.enumerate("model12", model12);
        }
        /* particle */
        __dna_docout.enumerate("particle", particle);
        /* unknown1 */
        __dna_docout.writeFloat("unknown1", unknown1);
        /* unknown2 */
        __dna_docout.writeFloat("unknown2", unknown2);
        /* unknown3 */
        __dna_docout.writeFloat("unknown3", unknown3);
        /* unknown4 */
        __dna_docout.writeFloat("unknown4", unknown4);
        /* wpsc1 */
        __dna_docout.enumerate("wpsc1", wpsc1);
        /* damageInfo1 */
        __dna_docout.enumerate("damageInfo1", damageInfo1);
        /* ridleyStruct1 */
        __dna_docout.enumerate("ridleyStruct1", ridleyStruct1);
        /* soundID1 */
        __dna_docout.writeUint32("soundID1", soundID1);
        /* wpsc2 */
        __dna_docout.enumerate("wpsc2", wpsc2);
        /* wpsc3 */
        if (propertyCount == 40)
            __dna_docout.enumerate("wpsc3", wpsc3);
        /* damageInfo2 */
        __dna_docout.enumerate("damageInfo2", damageInfo2);
        /* ridleyStruct2_1 */
        __dna_docout.enumerate("ridleyStruct2_1", ridleyStruct2_1);
        /* wpsc4 */
        __dna_docout.enumerate("wpsc4", wpsc4);
        /* damageInfo3 */
        __dna_docout.enumerate("damageInfo3", damageInfo3);
        /* ridleyStruct2_2 */
        __dna_docout.enumerate("ridleyStruct2_2", ridleyStruct2_2);
        /* soundID2 */
        __dna_docout.writeUint32("soundID2", soundID2);
        /* damageInfo4 */
        __dna_docout.enumerate("damageInfo4", damageInfo4);
        /* ridleyStruct2_3 */
        __dna_docout.enumerate("ridleyStruct2_3", ridleyStruct2_3);
        /* unknown5 */
        __dna_docout.writeFloat("unknown5", unknown5);
        /* unknown6 */
        __dna_docout.writeFloat("unknown6", unknown6);
        /* damageInfo5 */
        __dna_docout.enumerate("damageInfo5", damageInfo5);
        /* unknown7 */
        __dna_docout.writeFloat("unknown7", unknown7);
        /* damageInfo6 */
        __dna_docout.enumerate("damageInfo6", damageInfo6);
        /* unknown8 */
        __dna_docout.writeFloat("unknown8", unknown8);
        /* damageInfo7 */
        __dna_docout.enumerate("damageInfo7", damageInfo7);
        /* unknown9 */
        __dna_docout.writeFloat("unknown9", unknown9);
        /* elsc */
        __dna_docout.enumerate("elsc", elsc);
        /* unknown10 */
        __dna_docout.writeFloat("unknown10", unknown10);
        /* soundID3 */
        __dna_docout.writeUint32("soundID3", soundID3);
        /* damageInfo8 */
        __dna_docout.enumerate("damageInfo8", damageInfo8);

        if (propertyCount == 40)
            __dna_docout.enumerate("damageInfo9", damageInfo9);
    }

    static const char* DNAType()
    {
        return "Retro::DNAMP1::Ridley";
    }

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
            ent->name = name + "_part";
        }
        if (model1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model1);
            ent->name = name + "_model1";
        }
        if (model2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model2);
            ent->name = name + "_model2";
        }
        if (model3)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model3);
            ent->name = name + "_model3";
        }
        if (model4)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model4);
            ent->name = name + "_model4";
        }
        if (model5)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model5);
            ent->name = name + "_model5";
        }
        if (model6)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model6);
            ent->name = name + "_model6";
        }
        if (model7)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model7);
            ent->name = name + "_model7";
        }
        if (model8)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model8);
            ent->name = name + "_model8";
        }
        if (model9)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model9);
            ent->name = name + "_model9";
        }
        if (model10)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model10);
            ent->name = name + "_model10";
        }
        if (model11)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model11);
            ent->name = name + "_model11";
        }
        if (model12)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model12);
            ent->name = name + "_model12";
        }
        if (wpsc1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc1);
            ent->name = name + "_wpsc1";
        }
        if (wpsc2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc2);
            ent->name = name + "_wpsc2";
        }
        if (wpsc3)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc3);
            ent->name = name + "_wpsc3";
        }
        if (wpsc4)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc4);
            ent->name = name + "_wpsc4";
        }
        if (elsc)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(elsc);
            ent->name = name + "_elsc";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
        ridleyStruct1.nameIDs(pakRouter, name + "_ridley1");
    }
};
}
}

#endif
