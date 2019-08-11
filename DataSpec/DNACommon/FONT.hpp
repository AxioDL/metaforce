#pragma once

#include "PAK.hpp"
#include "athena/FileWriter.hpp"

namespace DataSpec::DNAFont {
struct GlyphRect : BigDNA {
  AT_DECL_DNA_YAML
  Value<float> left;
  Value<float> top;
  Value<float> right;
  Value<float> bottom;
};
struct IGlyph : BigDNAVYaml {
  AT_DECL_DNA_YAMLV
  Value<atUint16> m_character;
  GlyphRect m_glyphRect;

  atUint16 character() const { return m_character; }
  float left() const { return m_glyphRect.left; }
  float top() const { return m_glyphRect.top; }
  float right() const { return m_glyphRect.right; }
  float bottom() const { return m_glyphRect.bottom; }
  GlyphRect rect() const { return m_glyphRect; }

  virtual atInt32 layer() const { return 0; }
  virtual atInt32 leftPadding() const = 0;
  virtual atInt32 advance() const = 0;
  virtual atInt32 rightPadding() const = 0;
  virtual atInt32 width() const = 0;
  virtual atInt32 height() const = 0;
  virtual atInt32 baseline() const = 0;
  virtual atInt32 kerningIndex() const = 0;
};

struct GlyphMP1 : IGlyph {
  AT_DECL_DNA_YAMLV
  Value<atInt32> m_leftPadding;
  Value<atInt32> m_advance;
  Value<atInt32> m_rightPadding;
  Value<atInt32> m_width;
  Value<atInt32> m_height;
  Value<atInt32> m_baseline;
  Value<atInt32> m_kerningIndex;

  atInt32 leftPadding() const override { return m_leftPadding; }
  atInt32 advance() const override { return m_advance; }
  atInt32 rightPadding() const override { return m_rightPadding; }
  atInt32 width() const override { return m_width; }
  atInt32 height() const override { return m_height; }
  atInt32 baseline() const override { return m_baseline; }
  atInt32 kerningIndex() const override { return m_kerningIndex; }
};

struct GlyphMP2 : IGlyph {
  AT_DECL_DNA_YAMLV
  Value<atInt8> m_layer;
  Value<atInt8> m_leftPadding;
  Value<atInt8> m_advance;
  Value<atInt8> m_rightPadding;
  Value<atInt8> m_width;
  Value<atInt8> m_height;
  Value<atInt8> m_baseline;
  Value<atInt16> m_kerningIndex;

  atInt32 layer() const override { return m_layer; }
  atInt32 leftPadding() const override { return m_leftPadding; }
  atInt32 advance() const override { return m_advance; }
  atInt32 rightPadding() const override { return m_rightPadding; }
  atInt32 width() const override { return m_width; }
  atInt32 height() const override { return m_height; }
  atInt32 baseline() const override { return m_baseline; }
  atInt32 kerningIndex() const override { return m_kerningIndex; }
};

struct KerningInfo : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint16> thisChar;
  Value<atUint16> nextChar;
  Value<atInt32> adjust;
};

template <class IDType>
struct AT_SPECIALIZE_PARMS(DataSpec::UniqueID32, DataSpec::UniqueID64) FONT : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  AT_SUBDECL_DNA
  Value<atUint32> version;
  Value<atUint32> unknown1;
  Value<atInt32> lineHeight;
  Value<atInt32> verticalOffset;
  Value<atInt32> lineMargin;
  Value<bool> unknown2;
  Value<bool> unknown3;
  Value<atUint32> unknown4;
  Value<atUint32> fontSize; // in points
  String<-1> name;
  Value<IDType> textureId;
  Value<atUint32> textureFormat;
  Value<atUint32> glyphCount;
  std::vector<std::unique_ptr<IGlyph>> glyphs;
  Value<atUint32> kerningInfoCount;
  Vector<KerningInfo, AT_DNA_COUNT(kerningInfoCount)> kerningInfo;

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const {
    g_curSpec->flattenDependencies(textureId, pathsOut);
  }
};

template <class IDType>
bool ExtractFONT(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteFONT(const FONT<IDType>& font, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAFont
