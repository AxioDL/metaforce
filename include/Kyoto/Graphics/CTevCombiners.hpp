#ifndef _CTEVCOMBINERS
#define _CTEVCOMBINERS

#include "types.h"

#include <dolphin/gx/GXEnum.h>

class CTevCombiners {
public:
  enum EColorSrc {
    kCS_PreviousColor,
    kCS_PreviousAlpha,
    kCS_RegisterC0,
    kCS_RegisterA0,
    kCS_RegisterC1,
    kCS_RegisterA1,
    kCS_RegisterC2,
    kCS_RegisterA2,
    kCS_TextureColor,
    kCS_TextureAlpha,
    kCS_RasterColor,
    kCS_RasterAlpha,
    kCS_One,
    kCS_Half,
    kCS_Konst,
    kCS_Zero,
  };

  class ColorVar {
  public:
    ColorVar(EColorSrc src);
    ColorVar(const ColorVar& other) : x0_src(other.GetSource()) {}

    EColorSrc GetSource() const { return x0_src; }

  private:
    EColorSrc x0_src;
  };

  class ColorPass {
  public:
    ColorPass(const ColorVar& a, const ColorVar& b, const ColorVar& c, const ColorVar& d)
    : x0_a(a), x4_b(b), x8_c(c), xc_d(d) {}

    ColorPass(const ColorPass& other)
    : x0_a(other.x0_a), x4_b(other.x4_b), x8_c(other.x8_c), xc_d(other.xc_d) {}

    ColorVar GetA() const { return x0_a; }
    ColorVar GetB() const { return x4_b; }
    ColorVar GetC() const { return x8_c; }
    ColorVar GetD() const { return xc_d; }

  private:
    ColorVar x0_a;
    ColorVar x4_b;
    ColorVar x8_c;
    ColorVar xc_d;
  };

  enum EAlphaSrc {
    kAS_PreviousAlpha,
    kAS_RegisterA0,
    kAS_RegisterA1,
    kAS_RegisterA2,
    kAS_TextureAlpha,
    kAS_RasterAlpha,
    kAS_Konst,
    kAS_Zero,
  };

  class AlphaVar {
  public:
    AlphaVar(EAlphaSrc src);
    AlphaVar(const AlphaVar& other) : x0_src(other.GetSource()) {}

    EAlphaSrc GetSource() const { return x0_src; }

  private:
    EAlphaSrc x0_src;
  };

  class AlphaPass {
  public:
    AlphaPass(const AlphaVar& a, const AlphaVar& b, const AlphaVar& c, const AlphaVar& d)
    : x0_a(a), x4_b(b), x8_c(c), xc_d(d) {}

    AlphaPass(const AlphaPass& other)
    : x0_a(other.x0_a), x4_b(other.x4_b), x8_c(other.x8_c), xc_d(other.xc_d) {}

    AlphaVar GetA() const { return x0_a; }
    AlphaVar GetB() const { return x4_b; }
    AlphaVar GetC() const { return x8_c; }
    AlphaVar GetD() const { return xc_d; }

  private:
    AlphaVar x0_a;
    AlphaVar x4_b;
    AlphaVar x8_c;
    AlphaVar xc_d;
  };

  enum ETevOp {
    kTO_Add,
    kTO_Subtract,
  };

  enum ETevBias {
    kTB_Zero,
    kTB_AddHalf,
    kTB_SubHalf,
  };

  enum ETevScale {
    kTS_Scale1,
    kTS_Scale2,
    kTS_Scale4,
    kTS_Divide2,
  };

  enum ETevOutput {
    kTO_Previous,
    kTO_Register0,
    kTO_Register1,
    kTO_Register2,
  };

  class CTevOp {
  public:
    CTevOp(ETevOp op = kTO_Add, ETevBias bias = kTB_Zero, ETevScale scale = kTS_Scale1,
           bool clamp = true, ETevOutput output = kTO_Previous)
    : x0_clamp(clamp), x4_op(op), x8_bias(bias), xc_scale(scale), x10_output(output) {}
    CTevOp(const CTevOp& other)
    : x0_clamp(other.GetClamp())
    , x4_op(other.GetOp())
    , x8_bias(other.GetBias())
    , xc_scale(other.GetScale())
    , x10_output(other.GetOutput()) {}

    bool GetClamp() const { return x0_clamp; }
    ETevOp GetOp() const { return x4_op; }
    ETevBias GetBias() const { return x8_bias; }
    ETevScale GetScale() const { return xc_scale; }
    ETevOutput GetOutput() const { return x10_output; }

  private:
    bool x0_clamp;
    ETevOp x4_op;
    ETevBias x8_bias;
    ETevScale xc_scale;
    ETevOutput x10_output;
  };

  class CTevPass {
  public:
    CTevPass(const ColorPass& colorPass, const AlphaPass& alphaPass,
             const CTevOp& colorOp = CTevOp(), const CTevOp& alphaOp = CTevOp())
    : x0_id(sNextUniquePass++)
    , x4_colorPass(colorPass)
    , x14_alphaPass(alphaPass)
    , x24_colorOp(colorOp)
    , x38_alphaOp(alphaOp) {}

    void Execute(int) const;

  private:
    uint x0_id;
    ColorPass x4_colorPass;
    AlphaPass x14_alphaPass;
    CTevOp x24_colorOp;
    CTevOp x38_alphaOp;
  };

  static void RecomputePasses();
  static void Init();
  static void DeletePass(int);
  static void SetupPass(int, const CTevPass& pass);
  static bool SetPassCombiners(int stage, const CTevPass& pass);
  static void ResetStates();

  static int sNextUniquePass;
  static const CTevPass kEnvPassthru;
  static const AlphaVar skAlphaOne;
  static const ColorVar skColorOne;

private:
  static bool sValidPasses[2];
  static uint sNumEnabledPasses;
};

#endif // _CTEVCOMBINERS
