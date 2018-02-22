#include "FONT.hpp"

namespace DataSpec::DNAFont
{
logvisor::Module LogModule("urde::DNAFont");

template <class IDType>
void FONT<IDType>::_read(athena::io::IStreamReader& __dna_reader)
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
void FONT<IDType>::_write(athena::io::IStreamWriter& __dna_writer) const
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
void FONT<IDType>::_read(athena::io::YAMLDocReader& __dna_docin)
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
    if (auto v = __dna_docin.enterSubVector("glyphs", count))
    {
        glyphCount = count;
        for (atUint32 i = 0; i < glyphCount; i++)
        {
            if (version < 4)
                glyphs.emplace_back(new GlyphMP1);
            else
                glyphs.emplace_back(new GlyphMP2);

            if (auto rec = __dna_docin.enterSubRecord(nullptr))
                glyphs.back()->read(__dna_docin);
        }
    }
    /* kerningInfoCount squelched */
    /* kerningInfo */
    kerningInfoCount = __dna_docin.enumerate("kerningInfo", kerningInfo);
}

template <class IDType>
void FONT<IDType>::_write(athena::io::YAMLDocWriter& __dna_docout) const
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
    if (auto v = __dna_docout.enterSubVector("glyphs"))
        for (const std::unique_ptr<IGlyph>& glyph : glyphs)
            if (auto rec = __dna_docout.enterSubRecord(nullptr))
                glyph->write(__dna_docout);
    /* kerningInfoCount squelched */
    /* kerningInfo */
    __dna_docout.enumerate("kerningInfo", kerningInfo);
}

template <>
const char* FONT<UniqueID32>::DNAType()
{
    return "FONT<UniqueID32>";
}

template <>
const char* FONT<UniqueID64>::DNAType()
{
    return "FONT<UniqueID64>";
}

template <class IDType>
void FONT<IDType>::_binarySize(size_t& __isz) const
{
    __isz += name.size() + 1;
    textureId.binarySize(__isz);
    for (const std::unique_ptr<IGlyph>& glyph : glyphs)
        glyph->binarySize(__isz);
    for (const KerningInfo& k : kerningInfo)
        k.binarySize(__isz);
    __isz += 46;
}

AT_SUBSPECIALIZE_DNA_YAML(FONT<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(FONT<UniqueID64>)

template <class IDType>
bool ExtractFONT(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    if (writer.isOpen())
    {
        FONT<IDType> font;
        font.read(rs);
        athena::io::ToYAMLStream(font, writer);
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
            w.writeUByte(0xff);
    return true;
}
template bool WriteFONT<UniqueID32>(const FONT<UniqueID32>& font, const hecl::ProjectPath& outPath);
template bool WriteFONT<UniqueID64>(const FONT<UniqueID64>& font, const hecl::ProjectPath& outPath);

}
