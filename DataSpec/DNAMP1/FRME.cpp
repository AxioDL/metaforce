#include "FRME.hpp"

namespace Retro
{
namespace DNAMP1
{
void FRME::read(Athena::io::IStreamReader& __dna_reader)
{
    /* version */
    version = __dna_reader.readUint32Big();
    /* unk1 */
    unk1 = __dna_reader.readUint32Big();
    /* modelCount */
    modelCount = __dna_reader.readUint32Big();
    /* unk3 */
    unk3 = __dna_reader.readUint32Big();
    /* widgetCount */
    widgetCount = __dna_reader.readUint32Big();
    /* widgets */
    __dna_reader.enumerate<Widget>(widgets, widgetCount, [this](Athena::io::IStreamReader& reader, Widget& w) {
        w.owner = this;
        w.read(reader);
    });
}

void FRME::write(Athena::io::IStreamWriter& __dna_writer) const
{
    /* version */
    __dna_writer.writeUint32Big(version);
    /* unk1 */
    __dna_writer.writeUint32Big(unk1);
    /* modelCount */
    __dna_writer.writeUint32Big(modelCount);
    /* unk3 */
    __dna_writer.writeUint32Big(unk3);
    /* widgetCount */
    __dna_writer.writeUint32Big(widgetCount);
    /* widgets */
    __dna_writer.enumerate(widgets);
}

size_t FRME::binarySize(size_t __isz) const
{
    __isz = __EnumerateSize(__isz, widgets);
    return __isz + 20;
}

void FRME::Widget::read(Athena::io::IStreamReader& __dna_reader)
{
    /* type */
    type.read(__dna_reader);
    /* header */
    header.read(__dna_reader);
    switch(type)
    {
    case SBIG('BWIG'): widgetInfo.reset(new BWIGInfo); break;
    case SBIG('HWIG'): widgetInfo.reset(new HWIGInfo); break;
    case SBIG('CAMR'): widgetInfo.reset(new CAMRInfo); break;
    case SBIG('LITE'): widgetInfo.reset(new LITEInfo); break;
    case SBIG('ENRG'): widgetInfo.reset(new ENRGInfo); break;
    case SBIG('MODL'): widgetInfo.reset(new MODLInfo); break;
    case SBIG('METR'): widgetInfo.reset(new METRInfo); break;
    case SBIG('GRUP'): widgetInfo.reset(new GRUPInfo); break;
    case SBIG('TXPN'): widgetInfo.reset(new TXPNInfo(owner->version)); break;
    case SBIG('IMGP'): widgetInfo.reset(new IMGPInfo); break;
    case SBIG('TBGP'): widgetInfo.reset(new TBGPInfo); break;
    case SBIG('SLGP'): widgetInfo.reset(new SLGPInfo); break;
    default:
        Log.report(LogVisor::FatalError, _S("Unsupported FRME widget type %.8X"), type.toUint32());
    }

    /* widgetInfo */
    widgetInfo->read(__dna_reader);

    /* hasGroup */
    hasGroup = __dna_reader.readBool();
    if (hasGroup)
    {
        /* group */
        group = __dna_reader.readUint16Big();
    }
    /* origin */
    origin = __dna_reader.readVec3fBig();
    /* basis[0] */
    basis[0] = __dna_reader.readVec3fBig();
    /* basis[1] */
    basis[1] = __dna_reader.readVec3fBig();
    /* basis[2] */
    basis[2] = __dna_reader.readVec3fBig();
    /* rotationCenter */
    rotationCenter = __dna_reader.readVec3fBig();
    /* unk2 */
    unk2 = __dna_reader.readUint32Big();
    /* unk3 */
    unk3 = __dna_reader.readBool();
    /* unk4 */
    unk4 = __dna_reader.readBool();
}

void FRME::Widget::write(Athena::io::IStreamWriter& __dna_writer) const
{
    /* type */
    type.write(__dna_writer);
    /* header */
    header.write(__dna_writer);

    /* widgetInfo */
    widgetInfo->write(__dna_writer);

    /* hasGroup */
    __dna_writer.writeBool(hasGroup);
    if (hasGroup)
    {
        /* group */
        __dna_writer.writeUint16Big(group);
    }
    /* origin */
    __dna_writer.writeVec3fBig(origin);
    /* basis[0] */
    __dna_writer.writeVec3fBig(basis[0]);
    /* basis[1] */
    __dna_writer.writeVec3fBig(basis[1]);
    /* basis[2] */
    __dna_writer.writeVec3fBig(basis[2]);
    /* rotationCenter */
    __dna_writer.writeVec3fBig(rotationCenter);
    /* unk2 */
    __dna_writer.writeUint32Big(unk2);
    /* unk3 */
    __dna_writer.writeBool(unk3);
    /* unk4 */
    __dna_writer.writeBool(unk4);
}

size_t FRME::Widget::binarySize(size_t __isz) const
{
    __isz = type.binarySize(__isz);
    __isz = header.binarySize(__isz);
    __isz = widgetInfo->binarySize(__isz);
    if (hasGroup)
        __isz += 4;
    return __isz + 67;
}

void FRME::Widget::CAMRInfo::read(Athena::io::IStreamReader& __dna_reader)
{
    projectionType = ProjectionType(__dna_reader.readUint32Big());
    if (projectionType == ProjectionType::Perspective)
        projection.reset(new PerspectiveProjection);
    else if (projectionType == ProjectionType::Orthographic)
        projection.reset(new OrthographicProjection);
    else
        Log.report(LogVisor::FatalError, _S("Invalid CAMR projection mode! %i"), int(projectionType));

    projection->read(__dna_reader);
}

void FRME::Widget::CAMRInfo::write(Athena::io::IStreamWriter& __dna_writer) const
{
    if (!projection)
        Log.report(LogVisor::FatalError, _S("Invalid CAMR projection object!"));
    if (projection->type != projectionType)
        Log.report(LogVisor::FatalError, _S("CAMR projection type does not match actual projection type!"));

    __dna_writer.writeUint32Big(atUint32(projectionType));
    projection->write(__dna_writer);
}

size_t FRME::Widget::CAMRInfo::binarySize(size_t __isz) const
{
    __isz = projection->binarySize(__isz);
    return __isz + 4;
}

void FRME::Widget::TXPNInfo::read(Athena::io::IStreamReader& __dna_reader)
{
    IWidgetInfo::read(__dna_reader);
    /* frameVals[0] */
    frameVals[0] = __dna_reader.readFloatBig();
    /* frameVals[1] */
    frameVals[1] = __dna_reader.readFloatBig();
    /* frameVals[2] */
    frameVals[2] = __dna_reader.readFloatBig();
    /* frameVals[3] */
    frameVals[3] = __dna_reader.readFloatBig();
    /* frameVals[4] */
    frameVals[4] = __dna_reader.readFloatBig();
    /* font */
    font.read(__dna_reader);
    /* unk1 */
    unk1 = __dna_reader.readBool();
    /* unk2 */
    unk2 = __dna_reader.readBool();
    /* unk3 */
    unk3 = __dna_reader.readUint32Big();
    /* unk4 */
    unk4 = __dna_reader.readUint32Big();
    /* fillColor */
    fillColor = __dna_reader.readVec4fBig();
    /* outlineColor */
    outlineColor = __dna_reader.readVec4fBig();
    /* pointScale */
    pointScale = __dna_reader.readVec2fBig();
    if (version == 1)
    {
        /* jpnFont */
        jpnFont.read(__dna_reader);
        /* jpnPointScale[0] */
        jpnPointScale[0] = __dna_reader.readInt32Big();
        /* jpnPointScale[0] */
        jpnPointScale[1] = __dna_reader.readInt32Big();
    }
}

void FRME::Widget::TXPNInfo::write(Athena::io::IStreamWriter& __dna_writer) const
{
    IWidgetInfo::write(__dna_writer);
    /* frameVals[0] */
    __dna_writer.writeFloatBig(frameVals[0]);
    /* frameVals[1] */
    __dna_writer.writeFloatBig(frameVals[1]);
    /* frameVals[2] */
    __dna_writer.writeFloatBig(frameVals[2]);
    /* frameVals[3] */
    __dna_writer.writeFloatBig(frameVals[3]);
    /* frameVals[4] */
    __dna_writer.writeFloatBig(frameVals[4]);
    /* font */
    font.write(__dna_writer);
    /* unk1 */
    __dna_writer.writeBool(unk1);
    /* unk2 */
    __dna_writer.writeBool(unk2);
    /* unk3 */
    __dna_writer.writeUint32Big(unk3);
    /* unk4 */
    __dna_writer.writeUint32Big(unk4);
    /* fillColor */
    __dna_writer.writeVec4fBig(fillColor);
    /* outlineColor */
    __dna_writer.writeVec4fBig(outlineColor);
    /* pointScale */
    __dna_writer.writeVec2fBig(pointScale);
    if (version == 1)
    {
        /* jpnFont */
        jpnFont.write(__dna_writer);
        /* jpnPointScale[0] */
        __dna_writer.writeInt32Big(jpnPointScale[0]);
        /* jpnPointScale[1] */
        __dna_writer.writeInt32Big(jpnPointScale[1]);
    }
}

size_t FRME::Widget::TXPNInfo::binarySize(size_t __isz) const
{
    __isz = IWidgetInfo::binarySize(__isz);
    __isz = font.binarySize(__isz);
    if (version == 1)
        __isz = jpnFont.binarySize(__isz);

    return __isz + (version == 1 ? 78 : 66);
}

}
}
