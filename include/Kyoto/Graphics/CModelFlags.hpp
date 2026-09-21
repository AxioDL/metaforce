#ifndef _CMODELFLAGS
#define _CMODELFLAGS

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"

class CModelFlags {
public:
  enum ETrans {
    kT_Opaque = 0,
    kT_One = 1,   // ?
    kT_Two = 2,   // ?
    kT_Three = 3, // ?
    kT_Four = 4,  // ?
    kT_Blend = 5,
    kT_Additive = 7,
    kT_Additive2 = 8,
  };
  enum EFlags {
    kF_Nothing = 0,
    kF_DepthCompare = 0x1,
    kF_DepthUpdate = 0x2,
    kF_NoTextureLock = 0x4,
    kF_DepthGreater = 0x8,
    kF_DepthNonInclusive = 0x10,
    kF_DrawNormal = 0x20,
    kF_ThermalUnsortedOnly = 0x40,
  };

  CModelFlags(const ETrans trans, const float rgba)
  : x0_blendMode(trans)
  , x1_matSetIdx(0)
  , x2_flags(kF_DepthCompare | kF_DepthUpdate)
  , x4_color(1.f, 1.f, 1.f, rgba) {}
  CModelFlags(const ETrans trans, const CColor& color)
  : x0_blendMode(trans)
  , x1_matSetIdx(0)
  , x2_flags(kF_DepthCompare | kF_DepthUpdate)
  , x4_color(color) {}

  CModelFlags(const ETrans blendMode, const uchar shadIdx, const EFlags flags, const CColor& col)
  : x0_blendMode(blendMode), x1_matSetIdx(shadIdx), x2_flags(flags), x4_color(col) {}

  CModelFlags(const CModelFlags& flags, const uint otherFlags)
  : x0_blendMode(flags.x0_blendMode)
  , x1_matSetIdx(flags.x1_matSetIdx)
  , x2_flags(otherFlags)
  , x4_color(flags.x4_color) {}
  CModelFlags(const CModelFlags& flags, const bool b /* TODO what's this? */, const int shaderSet)
  : x0_blendMode(flags.GetBlendMode())
  , x1_matSetIdx(shaderSet)
  , x2_flags(flags.x2_flags)
  , x4_color(flags.x4_color) {}

  // CModelFlags(const CModelFlags& other)
  // : x0_blendMode(other.x0_blendMode)
  // , x1_matSetIdx(other.x1_matSetIdx)
  // , x2_flags(other.x2_flags)
  // , x4_color(other.x4_color) {}
  CModelFlags& operator=(const CModelFlags& other) {
    x0_blendMode = other.GetBlendMode();
    x1_matSetIdx = other.x1_matSetIdx;
    x2_flags = other.GetOtherFlags();
    x4_color = other.GetColor();
    return *this;
  }

  CModelFlags UseShaderSet(int matSet) const { return CModelFlags(*this, false, matSet); }
  CModelFlags DontLoadTextures() const {
    return CModelFlags(*this, GetOtherFlags() | kF_NoTextureLock);
  }
  CModelFlags DepthCompareUpdate(const bool compare, const bool update) const {
    const uint newFlags = static_cast< uint >(compare) | (static_cast< uint >(update) << 1);
    return CModelFlags(*this, (x2_flags & ~(kF_DepthCompare | kF_DepthUpdate)) | newFlags);
  }
  CModelFlags DepthBackwards() const {
    const EFlags flags = static_cast< EFlags >(GetOtherFlags() | kF_DepthGreater);
    return CModelFlags(static_cast< ETrans >(x0_blendMode), GetShaderSet(), flags, x4_color);
  }

  const uchar GetBlendMode() const { return x0_blendMode; }
  const ETrans GetTrans() const { return static_cast< ETrans >(x0_blendModeChar); }
  const int GetShaderSet() const { return x1_matSetIdxChar; }
  const int GetOtherFlags() const { return x2_flags; }
  const CColor GetColor() const { return x4_color; }
  const CColor& GetColorRef() const { return x4_color; }

  bool operator==(const CModelFlags& other) const {
    // TODO: cast to char for extsb; see CScriptActor::PreRender
    return static_cast< char >(x0_blendMode) == static_cast< char >(other.x0_blendMode) &&
           static_cast< char >(x1_matSetIdx) == static_cast< char >(other.x1_matSetIdx) &&
           x2_flags == other.x2_flags && x4_color == other.x4_color;
  }

  static CModelFlags Normal() { return CModelFlags(kT_Opaque, 1.f); }
  static CModelFlags AlphaBlended(const float alpha) { return CModelFlags(kT_Blend, alpha); }
  static CModelFlags AlphaBlended(const CColor& color) { return CModelFlags(kT_Blend, color); }
  static CModelFlags AlphaBlendedDepthCompareUpdate(const float alpha, const bool compare,
                                                    const bool update) {
    return AlphaBlended(alpha).DepthCompareUpdate(compare, update);
  }
  static CModelFlags AlphaBlendedDepthCompareUpdate(const CColor& color, const bool compare,
                                                    const bool update) {
    return AlphaBlended(color).DepthCompareUpdate(compare, update);
  }
  static CModelFlags Additive(const float f) { return CModelFlags(kT_Additive, f); }
  static CModelFlags Additive(const CColor& color) { return CModelFlags(kT_Additive, color); }
  static CModelFlags AdditiveDepthCompareUpdate(const CColor& color, const bool compare,
                                                const bool update) {
    return Additive(color).DepthCompareUpdate(compare, update);
  }
  static CModelFlags AdditiveRGB(const CColor& color) { return CModelFlags(kT_Additive2, color); }
  static CModelFlags ColorModulate(const CColor& color) { return CModelFlags(kT_One, color); }

public:
  // Dumb hack, need to figure this out
  union {
    uchar x0_blendMode;
    char x0_blendModeChar;
  };
  union {
    uchar x1_matSetIdx;
    char x1_matSetIdxChar;
  };
  ushort x2_flags;
  CColor x4_color;
};
CHECK_SIZEOF(CModelFlags, 0x8)

#endif // _CMODELFLAGS
