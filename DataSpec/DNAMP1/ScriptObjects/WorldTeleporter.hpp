#ifndef _DNAMP1_WORLDTELEPORTER_HPP_
#define _DNAMP1_WORLDTELEPORTER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct WorldTeleporter : IScriptObject
{
    Delete expl;
    String<-1> name;
    Value<bool> unknown1;
    UniqueID32 mlvl;
    UniqueID32 mrea;
    AnimationParameters animationParameters;
    Value<atVec3f> unknown2;
    UniqueID32 model1;
    Value<atVec3f> unknown3;
    UniqueID32 model2;
    Value<atVec3f> unknown4;
    Value<bool> unknown5;
    Value<atUint32> soundID; // needs verifcation
    Value<atUint32> unknown6;
    Value<atUint32> unknown7;
    Value<bool> unknown8;
    UniqueID32 font;
    UniqueID32 strg;
    Value<bool> unknown9;
    Value<float> unknown10;
    Value<float> unknown11;
    Value<float> unknown12;
    
    /* Trilogy additions (property count 26) */
    String<-1> audioStream;
    Value<bool> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    Value<float> unknown16;
    
    void read(athena::io::IStreamReader& __dna_reader)
    {
        IScriptObject::read(__dna_reader);
        /* name */
        name = __dna_reader.readString(-1);
        /* unknown1 */
        unknown1 = __dna_reader.readBool();
        /* mlvl */
        mlvl.read(__dna_reader);
        /* mrea */
        mrea.read(__dna_reader);
        /* animationParameters */
        animationParameters.read(__dna_reader);
        /* unknown2 */
        unknown2 = __dna_reader.readVec3fBig();
        /* model1 */
        model1.read(__dna_reader);
        /* unknown3 */
        unknown3 = __dna_reader.readVec3fBig();
        /* model2 */
        model2.read(__dna_reader);
        /* unknown4 */
        unknown4 = __dna_reader.readVec3fBig();
        /* unknown5 */
        unknown5 = __dna_reader.readBool();
        /* soundID */
        soundID = __dna_reader.readUint32Big();
        /* unknown6 */
        unknown6 = __dna_reader.readUint32Big();
        /* unknown7 */
        unknown7 = __dna_reader.readUint32Big();
        /* unknown8 */
        unknown8 = __dna_reader.readBool();
        /* font */
        font.read(__dna_reader);
        /* strg */
        strg.read(__dna_reader);
        /* unknown9 */
        unknown9 = __dna_reader.readBool();
        /* unknown10 */
        unknown10 = __dna_reader.readFloatBig();
        /* unknown11 */
        unknown11 = __dna_reader.readFloatBig();
        /* unknown12 */
        unknown12 = __dna_reader.readFloatBig();
        
        if (propertyCount == 26)
        {
            audioStream = __dna_reader.readString();
            unknown13 = __dna_reader.readBool();
            unknown14 = __dna_reader.readFloatBig();
            unknown15 = __dna_reader.readFloatBig();
            unknown16 = __dna_reader.readFloatBig();
        }
        else
        {
            unknown13 = false;
            unknown14 = 0.0;
            unknown15 = 0.0;
            unknown16 = 0.0;
        }
    }
    
    void write(athena::io::IStreamWriter& __dna_writer) const
    {
        IScriptObject::write(__dna_writer);
        /* name */
        __dna_writer.writeString(name, -1);
        /* unknown1 */
        __dna_writer.writeBool(unknown1);
        /* mlvl */
        mlvl.write(__dna_writer);
        /* mrea */
        mrea.write(__dna_writer);
        /* animationParameters */
        animationParameters.write(__dna_writer);
        /* unknown2 */
        __dna_writer.writeVec3fBig(unknown2);
        /* model1 */
        model1.write(__dna_writer);
        /* unknown3 */
        __dna_writer.writeVec3fBig(unknown3);
        /* model2 */
        model2.write(__dna_writer);
        /* unknown4 */
        __dna_writer.writeVec3fBig(unknown4);
        /* unknown5 */
        __dna_writer.writeBool(unknown5);
        /* soundID */
        __dna_writer.writeUint32Big(soundID);
        /* unknown6 */
        __dna_writer.writeUint32Big(unknown6);
        /* unknown7 */
        __dna_writer.writeUint32Big(unknown7);
        /* unknown8 */
        __dna_writer.writeBool(unknown8);
        /* font */
        font.write(__dna_writer);
        /* strg */
        strg.write(__dna_writer);
        /* unknown9 */
        __dna_writer.writeBool(unknown9);
        /* unknown10 */
        __dna_writer.writeFloatBig(unknown10);
        /* unknown11 */
        __dna_writer.writeFloatBig(unknown11);
        /* unknown12 */
        __dna_writer.writeFloatBig(unknown12);
        
        if (propertyCount == 26)
        {
            __dna_writer.writeString(audioStream);
            __dna_writer.writeBool(unknown13);
            __dna_writer.writeFloatBig(unknown14);
            __dna_writer.writeFloatBig(unknown15);
            __dna_writer.writeFloatBig(unknown16);
        }
    }

    size_t binarySize(size_t __isz) const
    {
        __isz = IScriptObject::binarySize(__isz);
        __isz += name.size() + 1;
        __isz = mlvl.binarySize(__isz);
        __isz = mrea.binarySize(__isz);
        __isz = animationParameters.binarySize(__isz);
        __isz = model1.binarySize(__isz);
        __isz = model2.binarySize(__isz);
        __isz = font.binarySize(__isz);
        __isz = strg.binarySize(__isz);
        if (propertyCount == 26)
        {
            __isz += audioStream.size() + 1;
            __isz += 13;
        }
        return __isz + 64;
    }
    
    void read(athena::io::YAMLDocReader& __dna_docin)
    {
        IScriptObject::read(__dna_docin);
        /* name */
        name = __dna_docin.readString("name");
        /* unknown1 */
        unknown1 = __dna_docin.readBool("unknown1");
        /* mlvl */
        __dna_docin.enumerate("mlvl", mlvl);
        /* mrea */
        __dna_docin.enumerate("mrea", mrea);
        /* animationParameters */
        __dna_docin.enumerate("animationParameters", animationParameters);
        /* unknown2 */
        unknown2 = __dna_docin.readVec3f("unknown2");
        /* model1 */
        __dna_docin.enumerate("model1", model1);
        /* unknown3 */
        unknown3 = __dna_docin.readVec3f("unknown3");
        /* model2 */
        __dna_docin.enumerate("model2", model2);
        /* unknown4 */
        unknown4 = __dna_docin.readVec3f("unknown4");
        /* unknown5 */
        unknown5 = __dna_docin.readBool("unknown5");
        /* soundID */
        soundID = __dna_docin.readUint32("soundID");
        /* unknown6 */
        unknown6 = __dna_docin.readUint32("unknown6");
        /* unknown7 */
        unknown7 = __dna_docin.readUint32("unknown7");
        /* unknown8 */
        unknown8 = __dna_docin.readBool("unknown8");
        /* font */
        __dna_docin.enumerate("font", font);
        /* strg */
        __dna_docin.enumerate("strg", strg);
        /* unknown9 */
        unknown9 = __dna_docin.readBool("unknown9");
        /* unknown10 */
        unknown10 = __dna_docin.readFloat("unknown10");
        /* unknown11 */
        unknown11 = __dna_docin.readFloat("unknown11");
        /* unknown12 */
        unknown12 = __dna_docin.readFloat("unknown12");
        
        if (propertyCount == 26)
        {
            audioStream = __dna_docin.readString("audioStream");
            unknown13 = __dna_docin.readBool("unknown13");
            unknown14 = __dna_docin.readFloat("unknown14");
            unknown15 = __dna_docin.readFloat("unknown15");
            unknown16 = __dna_docin.readFloat("unknown16");
        }
        else
        {
            unknown13 = false;
            unknown14 = 0.0;
            unknown15 = 0.0;
            unknown16 = 0.0;
        }
    }
    
    void write(athena::io::YAMLDocWriter& __dna_docout) const
    {
        IScriptObject::write(__dna_docout);
        /* name */
        __dna_docout.writeString("name", name);
        /* unknown1 */
        __dna_docout.writeBool("unknown1", unknown1);
        /* mlvl */
        __dna_docout.enumerate("mlvl", mlvl);
        /* mrea */
        __dna_docout.enumerate("mrea", mrea);
        /* animationParameters */
        __dna_docout.enumerate("animationParameters", animationParameters);
        /* unknown2 */
        __dna_docout.writeVec3f("unknown2", unknown2);
        /* model1 */
        __dna_docout.enumerate("model1", model1);
        /* unknown3 */
        __dna_docout.writeVec3f("unknown3", unknown3);
        /* model2 */
        __dna_docout.enumerate("model2", model2);
        /* unknown4 */
        __dna_docout.writeVec3f("unknown4", unknown4);
        /* unknown5 */
        __dna_docout.writeBool("unknown5", unknown5);
        /* soundID */
        __dna_docout.writeUint32("soundID", soundID);
        /* unknown6 */
        __dna_docout.writeUint32("unknown6", unknown6);
        /* unknown7 */
        __dna_docout.writeUint32("unknown7", unknown7);
        /* unknown8 */
        __dna_docout.writeBool("unknown8", unknown8);
        /* font */
        __dna_docout.enumerate("font", font);
        /* strg */
        __dna_docout.enumerate("strg", strg);
        /* unknown9 */
        __dna_docout.writeBool("unknown9", unknown9);
        /* unknown10 */
        __dna_docout.writeFloat("unknown10", unknown10);
        /* unknown11 */
        __dna_docout.writeFloat("unknown11", unknown11);
        /* unknown12 */
        __dna_docout.writeFloat("unknown12", unknown12);
        
        if (propertyCount == 26)
        {
            __dna_docout.writeString("audioStream", audioStream);
            __dna_docout.writeBool("unknown13", unknown13);
            __dna_docout.writeFloat("unknown14", unknown14);
            __dna_docout.writeFloat("unknown15", unknown15);
            __dna_docout.writeFloat("unknown16", unknown16);
        }
    }
    
    static const char* DNAType()
    {
        return "urde::DNAMP1::WorldTeleporter";
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
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
        if (strg)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(strg);
            ent->name = name + "_strg";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(model1, pathsOut);
        g_curSpec->flattenDependencies(model2, pathsOut);
        g_curSpec->flattenDependencies(strg, pathsOut);
    }
};

}
}

#endif
