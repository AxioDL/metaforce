#include "FONT.hpp"

namespace DataSpec
{
namespace DNAFont
{
logvisor::Module LogModule("urde::DNAFont");

void GlyphRect::read(athena::io::IStreamReader& __dna_reader)
{
    /* left */
    left = __dna_reader.readFloatBig();
    /* top */
    top = __dna_reader.readFloatBig();
    /* right */
    right = __dna_reader.readFloatBig();
    /* bottom */
    bottom = __dna_reader.readFloatBig();
}

void GlyphRect::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* left */
    __dna_writer.writeFloatBig(left);
    /* top */
    __dna_writer.writeFloatBig(top);
    /* right */
    __dna_writer.writeFloatBig(right);
    /* bottom */
    __dna_writer.writeFloatBig(bottom);
}

void GlyphRect::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* left */
    left = __dna_docin.readFloat("left");
    /* top */
    top = __dna_docin.readFloat("top");
    /* right */
    right = __dna_docin.readFloat("right");
    /* bottom */
    bottom = __dna_docin.readFloat("bottom");
}

void GlyphRect::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* left */
    __dna_docout.writeFloat("left", left);
    /* top */
    __dna_docout.writeFloat("top", top);
    /* right */
    __dna_docout.writeFloat("right", right);
    /* bottom */
    __dna_docout.writeFloat("bottom", bottom);
}

const char* GlyphRect::DNAType()
{
    return "GlyphRect";
}

size_t GlyphRect::binarySize(size_t __isz) const
{
    return __isz + 16;
}

template <class IDType>
void FONT<IDType>::read(athena::io::IStreamReader& __dna_reader)
{
    /* magic */
    atUint32 magic;
    __dna_reader.readBytesToBuf(&magic, 4);
    if (magic != SBIG('FONT'))
    {
        LogModule.report(logvisor::Fatal, "Invalid FONT magic '%s'", &magic);
        return;
    }
    /* version */
    version = __dna_reader.readUint32Big();
    /* unknown1 */
    unknown1 = __dna_reader.readUint32Big();
    /* lineHeight */
    lineHeight = __dna_reader.readInt32Big();
    /* verticalOffset */
    verticalOffset = __dna_reader.readInt32Big();
    /* lineMargin */
    lineMargin = __dna_reader.readInt32Big();
    /* unknown2 */
    unknown2 = __dna_reader.readBool();
    /* unknown3 */
    unknown3 = __dna_reader.readBool();
    /* unknown4 */
    unknown4 = __dna_reader.readUint32Big();
    /* fontSize */
    fontSize = __dna_reader.readUint32Big();
    /* name */
    name = __dna_reader.readString(-1);
    /* textureId */
    textureId.read(__dna_reader);
    /* textureFormat */
    textureFormat = __dna_reader.readUint32Big();
    /* glyphCount */
    glyphCount = __dna_reader.readUint32Big();
    /* glyphs */
    for (atUint32 i = 0; i < glyphCount; i++)
    {
        if (version < 4)
            glyphs.emplace_back(new GlyphMP1);
        else
            glyphs.emplace_back(new GlyphMP2);
        glyphs.back()->read(__dna_reader);
    }
    /* kerningInfoCount */
    kerningInfoCount = __dna_reader.readUint32Big();
    /* kerningInfo */
    __dna_reader.enumerate(kerningInfo, kerningInfoCount);
}

template <class IDType>
void FONT<IDType>::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* magic */
    __dna_writer.writeBytes((atInt8*)"FONT", 4);
    /* version */
    __dna_writer.writeUint32Big(version);
    /* unknown1 */
    __dna_writer.writeUint32Big(unknown1);
    /* lineHeight */
    __dna_writer.writeInt32Big(lineHeight);
    /* verticalOffset */
    __dna_writer.writeInt32Big(verticalOffset);
    /* lineMargin */
    __dna_writer.writeInt32Big(lineMargin);
    /* unknown2 */
    __dna_writer.writeBool(unknown2);
    /* unknown3 */
    __dna_writer.writeBool(unknown3);
    /* unknown4 */
    __dna_writer.writeUint32Big(unknown4);
    /* fontSize */
    __dna_writer.writeUint32Big(fontSize);
    /* name */
    __dna_writer.writeString(name, -1);
    /* textureId */
    textureId.write(__dna_writer);
    /* textureFormat */
    __dna_writer.writeUint32Big(textureFormat);
    /* glyphCount */
    __dna_writer.writeUint32Big(glyphCount);
    /* glyphs */
    for (const std::unique_ptr<IGlyph>& glyph : glyphs)
        glyph->write(__dna_writer);
    /* kerningInfoCount */
    __dna_writer.writeUint32Big(kerningInfoCount);
    /* kerningInfo */
    __dna_writer.enumerate(kerningInfo);
}

template <class IDType>
void FONT<IDType>::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* version */
    version = __dna_docin.readUint32("version");
    /* unknown1 */
    unknown1 = __dna_docin.readUint32("unknown1");
    /* lineHeight */
    lineHeight = __dna_docin.readInt32("lineHeight");
    /* verticalOffset */
    verticalOffset = __dna_docin.readInt32("verticalOffset");
    /* lineMargin */
    lineMargin = __dna_docin.readInt32("lineMargin");
    /* unknown2 */
    unknown2 = __dna_docin.readBool("unknown2");
    /* unknown3 */
    unknown3 = __dna_docin.readBool("unknown3");
    /* unknown4 */
    unknown4 = __dna_docin.readUint32("unknown4");
    /* fontSize */
    fontSize = __dna_docin.readUint32("fontSize");
    /* name */
    name = __dna_docin.readString("name");
    /* textureId */
    __dna_docin.enumerate("textureId", textureId);
    /* textureFormat */
    textureFormat = __dna_docin.readUint32("textureFormat");
    /* glyphCount */
    /* glyphs */
    size_t count;
    __dna_docin.enterSubVector("glyphs", count);
    glyphCount = count;
    for (atUint32 i = 0; i < glyphCount; i++)
    {
        if (version < 4)
            glyphs.emplace_back(new GlyphMP1);
        else
            glyphs.emplace_back(new GlyphMP2);

        __dna_docin.enterSubRecord(nullptr);
        glyphs.back()->read(__dna_docin);
        __dna_docin.leaveSubRecord();
    }
    __dna_docin.leaveSubVector();
    /* kerningInfoCount squelched */
    /* kerningInfo */
    kerningInfoCount = __dna_docin.enumerate("kerningInfo", kerningInfo);
}

template <class IDType>
void FONT<IDType>::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* version */
    __dna_docout.writeUint32("version", version);
    /* unknown1 */
    __dna_docout.writeUint32("unknown1", unknown1);
    /* lineHeight */
    __dna_docout.writeInt32("lineHeight", lineHeight);
    /* verticalOffset */
    __dna_docout.writeInt32("verticalOffset", verticalOffset);
    /* lineMargin */
    __dna_docout.writeInt32("lineMargin", lineMargin);
    /* unknown2 */
    __dna_docout.writeBool("unknown2", unknown2);
    /* unknown3 */
    __dna_docout.writeBool("unknown3", unknown3);
    /* unknown4 */
    __dna_docout.writeUint32("unknown4", unknown4);
    /* fontSize */
    __dna_docout.writeUint32("fontSize", fontSize);
    /* name */
    __dna_docout.writeString("name", name);
    /* textureId */
    __dna_docout.enumerate("textureId", textureId);
    /* textureFormat */
    __dna_docout.writeUint32("textureFormat", textureFormat);
    /* glyphCount squelched */
    /* glyphs */
    __dna_docout.enterSubVector("glyphs");

    for (const std::unique_ptr<IGlyph>& glyph : glyphs)
    {
        __dna_docout.enterSubRecord(nullptr);
        glyph->write(__dna_docout);
        __dna_docout.leaveSubRecord();
    }

    __dna_docout.leaveSubVector();
    /* kerningInfoCount squelched */
    /* kerningInfo */
    __dna_docout.enumerate("kerningInfo", kerningInfo);
}

template <class IDType>
const char* FONT<IDType>::DNAType()
{
    return "FONT";
}

template <class IDType>
size_t FONT<IDType>::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz = textureId.binarySize(__isz);
    for (const std::unique_ptr<IGlyph>& glyph : glyphs)
        __isz = glyph->binarySize(__isz);
    __isz = __EnumerateSize(__isz, kerningInfo);
    return __isz + 46;
}

void IGlyph::read(athena::io::IStreamReader& __dna_reader)
{
    /* m_character */
    m_character = __dna_reader.readUint16Big();
    /* m_glyphRect */
    m_glyphRect.read(__dna_reader);
}

void IGlyph::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* m_character */
    __dna_writer.writeUint16Big(m_character);
    /* m_glyphRect */
    m_glyphRect.write(__dna_writer);
}

void IGlyph::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* m_character */
    m_character = __dna_docin.readUint16("m_character");
    /* m_glyphRect */
    __dna_docin.enumerate("m_glyphRect", m_glyphRect);
}

void IGlyph::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* m_character */
    __dna_docout.writeUint16("m_character", m_character);
    /* m_glyphRect */
    __dna_docout.enumerate("m_glyphRect", m_glyphRect);
}

const char* IGlyph::DNAType()
{
    return "FONT::IGlyph";
}

size_t IGlyph::binarySize(size_t __isz) const
{
    __isz = m_glyphRect.binarySize(__isz);
    return __isz + 2;
}

void GlyphMP1::read(athena::io::IStreamReader& __dna_reader)
{
    IGlyph::read(__dna_reader);
    /* m_leftPadding */
    m_leftPadding = __dna_reader.readInt32Big();
    /* m_advance */
    m_advance = __dna_reader.readInt32Big();
    /* m_rightPadding */
    m_rightPadding = __dna_reader.readInt32Big();
    /* m_width */
    m_width = __dna_reader.readInt32Big();
    /* m_height */
    m_height = __dna_reader.readInt32Big();
    /* m_baseline */
    m_baseline = __dna_reader.readInt32Big();
    /* m_kerningIndex */
    m_kerningIndex = __dna_reader.readInt32Big();
}

void GlyphMP1::write(athena::io::IStreamWriter& __dna_writer) const
{
    IGlyph::write(__dna_writer);
    /* m_leftPadding */
    __dna_writer.writeInt32Big(m_leftPadding);
    /* m_advance */
    __dna_writer.writeInt32Big(m_advance);
    /* m_rightPadding */
    __dna_writer.writeInt32Big(m_rightPadding);
    /* m_width */
    __dna_writer.writeInt32Big(m_width);
    /* m_height */
    __dna_writer.writeInt32Big(m_height);
    /* m_baseline */
    __dna_writer.writeInt32Big(m_baseline);
    /* m_kerningIndex */
    __dna_writer.writeInt32Big(m_kerningIndex);
}

void GlyphMP1::read(athena::io::YAMLDocReader& __dna_docin)
{
    IGlyph::read(__dna_docin);
    /* m_leftPadding */
    m_leftPadding = __dna_docin.readInt32("m_leftPadding");
    /* m_advance */
    m_advance = __dna_docin.readInt32("m_advance");
    /* m_rightPadding */
    m_rightPadding = __dna_docin.readInt32("m_rightPadding");
    /* m_width */
    m_width = __dna_docin.readInt32("m_width");
    /* m_height */
    m_height = __dna_docin.readInt32("m_height");
    /* m_baseline */
    m_baseline = __dna_docin.readInt32("m_baseline");
    /* m_kerningIndex */
    m_kerningIndex = __dna_docin.readInt32("m_kerningIndex");
}

void GlyphMP1::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    IGlyph::write(__dna_docout);
    /* m_leftPadding */
    __dna_docout.writeInt32("m_leftPadding", m_leftPadding);
    /* m_advance */
    __dna_docout.writeInt32("m_advance", m_advance);
    /* m_rightPadding */
    __dna_docout.writeInt32("m_rightPadding", m_rightPadding);
    /* m_width */
    __dna_docout.writeInt32("m_width", m_width);
    /* m_height */
    __dna_docout.writeInt32("m_height", m_height);
    /* m_baseline */
    __dna_docout.writeInt32("m_baseline", m_baseline);
    /* m_kerningIndex */
    __dna_docout.writeInt32("m_kerningIndex", m_kerningIndex);
}

const char* GlyphMP1::DNAType()
{
    return "GlyphMP1";
}

size_t GlyphMP1::binarySize(size_t __isz) const
{
    __isz = IGlyph::binarySize(__isz);
    return __isz + 28;
}

void GlyphMP2::read(athena::io::IStreamReader& __dna_reader)
{
    IGlyph::read(__dna_reader);
    /* m_layer */
    m_layer = __dna_reader.readByte();
    /* m_leftPadding */
    m_leftPadding = __dna_reader.readByte();
    /* m_advance */
    m_advance = __dna_reader.readByte();
    /* m_rightPadding */
    m_rightPadding = __dna_reader.readByte();
    /* m_width */
    m_width = __dna_reader.readByte();
    /* m_height */
    m_height = __dna_reader.readByte();
    /* m_baseline */
    m_baseline = __dna_reader.readByte();
    /* m_kerningIndex */
    m_kerningIndex = __dna_reader.readInt16Big();
}

void GlyphMP2::write(athena::io::IStreamWriter& __dna_writer) const
{
    IGlyph::write(__dna_writer);
    /* m_layer */
    __dna_writer.writeByte(m_layer);
    /* m_leftPadding */
    __dna_writer.writeByte(m_leftPadding);
    /* m_advance */
    __dna_writer.writeByte(m_advance);
    /* m_rightPadding */
    __dna_writer.writeByte(m_rightPadding);
    /* m_width */
    __dna_writer.writeByte(m_width);
    /* m_height */
    __dna_writer.writeByte(m_height);
    /* m_baseline */
    __dna_writer.writeByte(m_baseline);
    /* m_kerningIndex */
    __dna_writer.writeInt16Big(m_kerningIndex);
}

void GlyphMP2::read(athena::io::YAMLDocReader& __dna_docin)
{
    IGlyph::read(__dna_docin);
    /* m_layer */
    m_layer = __dna_docin.readByte("m_layer");
    /* m_leftPadding */
    m_leftPadding = __dna_docin.readByte("m_leftPadding");
    /* m_advance */
    m_advance = __dna_docin.readByte("m_advance");
    /* m_rightPadding */
    m_rightPadding = __dna_docin.readByte("m_rightPadding");
    /* m_width */
    m_width = __dna_docin.readByte("m_width");
    /* m_height */
    m_height = __dna_docin.readByte("m_height");
    /* m_baseline */
    m_baseline = __dna_docin.readByte("m_baseline");
    /* m_kerningIndex */
    m_kerningIndex = __dna_docin.readInt16("m_kerningIndex");
}

void GlyphMP2::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    IGlyph::write(__dna_docout);
    /* m_layer */
    __dna_docout.writeByte("m_layer", m_layer);
    /* m_leftPadding */
    __dna_docout.writeByte("m_leftPadding", m_leftPadding);
    /* m_advance */
    __dna_docout.writeByte("m_advance", m_advance);
    /* m_rightPadding */
    __dna_docout.writeByte("m_rightPadding", m_rightPadding);
    /* m_width */
    __dna_docout.writeByte("m_width", m_width);
    /* m_height */
    __dna_docout.writeByte("m_height", m_height);
    /* m_baseline */
    __dna_docout.writeByte("m_baseline", m_baseline);
    /* m_kerningIndex */
    __dna_docout.writeInt16("m_kerningIndex", m_kerningIndex);
}

const char* GlyphMP2::DNAType()
{
    return "GlyphMP2";
}

size_t GlyphMP2::binarySize(size_t __isz) const
{
    __isz = IGlyph::binarySize(__isz);
    return __isz + 9;
}

void KerningInfo::read(athena::io::IStreamReader& __dna_reader)
{
    /* thisChar */
    thisChar = __dna_reader.readUint16Big();
    /* nextChar */
    nextChar = __dna_reader.readUint16Big();
    /* adjust */
    adjust = __dna_reader.readInt32Big();
}

void KerningInfo::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* thisChar */
    __dna_writer.writeUint16Big(thisChar);
    /* nextChar */
    __dna_writer.writeUint16Big(nextChar);
    /* adjust */
    __dna_writer.writeInt32Big(adjust);
}

void KerningInfo::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* thisChar */
    thisChar = __dna_docin.readUint16("thisChar");
    /* nextChar */
    nextChar = __dna_docin.readUint16("nextChar");
    /* adjust */
    adjust = __dna_docin.readInt32("adjust");
}

void KerningInfo::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* thisChar */
    __dna_docout.writeUint16("thisChar", thisChar);
    /* nextChar */
    __dna_docout.writeUint16("nextChar", nextChar);
    /* adjust */
    __dna_docout.writeInt32("adjust", adjust);
}

const char* KerningInfo::DNAType()
{
    return "KerningInfo";
}

size_t KerningInfo::binarySize(size_t __isz) const
{
    return __isz + 8;
}

template struct FONT<UniqueID32>;
template struct FONT<UniqueID64>;

template <class IDType>
bool ExtractFONT(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    if (writer.isOpen())
    {
        FONT<IDType> font;
        font.read(rs);
        font.toYAMLStream(writer);
        return true;
    }
    return false;
}

template bool ExtractFONT<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractFONT<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteFONT(const FONT<IDType>& font, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
    if (w.hasError())
        return false;
    font.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeBytes((atInt8*)"\xff", 1);
    return true;
}
template bool WriteFONT<UniqueID32>(const FONT<UniqueID32>& font, const hecl::ProjectPath& outPath);
template bool WriteFONT<UniqueID64>(const FONT<UniqueID64>& font, const hecl::ProjectPath& outPath);

}
}
