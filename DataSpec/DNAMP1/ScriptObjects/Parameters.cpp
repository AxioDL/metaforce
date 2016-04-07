#include "Parameters.hpp"
#include "../ANCS.hpp"

namespace DataSpec
{
namespace DNAMP1
{

UniqueID32 AnimationParameters::getCINF(PAKRouter<PAKBridge>& pakRouter) const
{
    if (!animationCharacterSet)
        return UniqueID32();
    const nod::Node* node;
    const PAK::Entry* ancsEnt = pakRouter.lookupEntry(animationCharacterSet, &node);
    ANCS ancs;
    {
        PAKEntryReadStream rs = ancsEnt->beginReadStream(*node);
        ancs.read(rs);
    }
    return ancs.characterSet.characters.at(character).cinf;
}
    
void ActorParameters::read(athena::io::IStreamReader& __dna_reader)
{
    /* propertyCount */
    propertyCount = __dna_reader.readUint32Big();
    /* lightParameters */
    lightParameters.read(__dna_reader);
    /* scannableParameters */
    scannableParameters.read(__dna_reader);
    /* xrayModel */
    xrayModel.read(__dna_reader);
    /* xraySkin */
    xraySkin.read(__dna_reader);
    /* thermalModel */
    thermalModel.read(__dna_reader);
    /* thermalSkin */
    thermalSkin.read(__dna_reader);
    /* unknown1 */
    unknown1 = __dna_reader.readBool();
    /* unknown2 */
    unknown2 = __dna_reader.readFloatBig();
    /* unknown3 */
    unknown3 = __dna_reader.readFloatBig();
    /* visorParameters */
    visorParameters.read(__dna_reader);
    /* thermalHeat */
    thermalHeat = __dna_reader.readBool();
    /* unknown4 */
    unknown4 = __dna_reader.readBool();
    /* unknown5 */
    unknown5 = __dna_reader.readBool();
    /* unknown6 */
    unknown6 = __dna_reader.readFloatBig();
}

void ActorParameters::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* propertyCount */
    __dna_writer.writeUint32Big(propertyCount);
    /* lightParameters */
    lightParameters.write(__dna_writer);
    /* scannableParameters */
    scannableParameters.write(__dna_writer);
    /* xrayModel */
    xrayModel.write(__dna_writer);
    /* xraySkin */
    xraySkin.write(__dna_writer);
    /* thermalModel */
    thermalModel.write(__dna_writer);
    /* thermalSkin */
    thermalSkin.write(__dna_writer);
    /* unknown1 */
    __dna_writer.writeBool(unknown1);
    /* unknown2 */
    __dna_writer.writeFloatBig(unknown2);
    /* unknown3 */
    __dna_writer.writeFloatBig(unknown3);
    /* visorParameters */
    visorParameters.write(__dna_writer);
    /* thermalHeat */
    __dna_writer.writeBool(thermalHeat);
    /* unknown4 */
    __dna_writer.writeBool(unknown4);
    /* unknown5 */
    __dna_writer.writeBool(unknown5);
    /* unknown6 */
    __dna_writer.writeFloatBig(unknown6);
}

void ActorParameters::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* propertyCount */
    propertyCount = __dna_docin.readUint32("propertyCount");
    /* lightParameters */
    __dna_docin.enumerate("lightParameters", lightParameters);
    /* scannableParameters */
    __dna_docin.enumerate("scannableParameters", scannableParameters);
    /* xrayModel */
    __dna_docin.enumerate("xrayModel", xrayModel);
    /* thermalModel */
    __dna_docin.enumerate("thermalModel", thermalModel);
    /* unknown1 */
    unknown1 = __dna_docin.readBool("unknown1");
    /* unknown2 */
    unknown2 = __dna_docin.readFloat("unknown2");
    /* unknown3 */
    unknown3 = __dna_docin.readFloat("unknown3");
    /* visorParameters */
    __dna_docin.enumerate("visorParameters", visorParameters);
    /* thermalHeat */
    thermalHeat = __dna_docin.readBool("thermalHeat");
    /* unknown4 */
    unknown4 = __dna_docin.readBool("unknown4");
    /* unknown5 */
    unknown5 = __dna_docin.readBool("unknown5");
    /* unknown6 */
    unknown6 = __dna_docin.readFloat("unknown6");
}

void ActorParameters::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* propertyCount */
    __dna_docout.writeUint32("propertyCount", propertyCount);
    /* lightParameters */
    __dna_docout.enumerate("lightParameters", lightParameters);
    /* scannableParameters */
    __dna_docout.enumerate("scannableParameters", scannableParameters);
    /* xrayModel */
    __dna_docout.enumerate("xrayModel", xrayModel);
    /* thermalModel */
    __dna_docout.enumerate("thermalModel", thermalModel);
    /* unknown1 */
    __dna_docout.writeBool("unknown1", unknown1);
    /* unknown2 */
    __dna_docout.writeFloat("unknown2", unknown2);
    /* unknown3 */
    __dna_docout.writeFloat("unknown3", unknown3);
    /* visorParameters */
    __dna_docout.enumerate("visorParameters", visorParameters);
    /* thermalHeat */
    __dna_docout.writeBool("thermalHeat", thermalHeat);
    /* unknown4 */
    __dna_docout.writeBool("unknown4", unknown4);
    /* unknown5 */
    __dna_docout.writeBool("unknown5", unknown5);
    /* unknown6 */
    __dna_docout.writeFloat("unknown6", unknown6);
}

const char* ActorParameters::DNAType()
{
    return "ActorParameters";
}

size_t ActorParameters::binarySize(size_t __isz) const
{
    __isz = lightParameters.binarySize(__isz);
    __isz = scannableParameters.binarySize(__isz);
    __isz = xrayModel.binarySize(__isz);
    __isz = xraySkin.binarySize(__isz);
    __isz = thermalModel.binarySize(__isz);
    __isz = thermalSkin.binarySize(__isz);
    __isz = visorParameters.binarySize(__isz);
    return __isz + 20;
}

}
}
