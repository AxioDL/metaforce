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
    ColorVar(const ColorVar& other) : mSrc(other.GetSource()) {}

    EColorSrc GetSource() const { return mSrc; }

  private:
    EColorSrc mSrc;
  };

  class ColorPass {
  public:
    ColorPass(const ColorVar& a, const ColorVar& b, const ColorVar& c, const ColorVar& d)
    : mA(a), mB(b), mC(c), mD(d) {}

    ColorPass(const ColorPass& other)
    : mA(other.mA), mB(other.mB), mC(other.mC), mD(other.mD) {}

    ColorVar GetA() const { return mA; }
    ColorVar GetB() const { return mB; }
    ColorVar GetC() const { return mC; }
    ColorVar GetD() const { return mD; }

  private:
    ColorVar mA;
    ColorVar mB;
    ColorVar mC;
    ColorVar mD;
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
    AlphaVar(const AlphaVar& other) : mSrc(other.GetSource()) {}

    EAlphaSrc GetSource() const { return mSrc; }

  private:
    EAlphaSrc mSrc;
  };

  class AlphaPass {
  public:
    AlphaPass(const AlphaVar& a, const AlphaVar& b, const AlphaVar& c, const AlphaVar& d)
    : mA(a), mB(b), mC(c), mD(d) {}

    AlphaPass(const AlphaPass& other)
    : mA(other.mA), mB(other.mB), mC(other.mC), mD(other.mD) {}

    AlphaVar GetA() const { return mA; }
    AlphaVar GetB() const { return mB; }
    AlphaVar GetC() const { return mC; }
    AlphaVar GetD() const { return mD; }

  private:
    AlphaVar mA;
    AlphaVar mB;
    AlphaVar mC;
    AlphaVar mD;
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
    : mClamp(clamp), mOp(op), mBias(bias), mScale(scale), mOutput(output) {}
    CTevOp(const CTevOp& other)
    : mClamp(other.GetClamp())
    , mOp(other.GetOp())
    , mBias(other.GetBias())
    , mScale(other.GetScale())
    , mOutput(other.GetOutput()) {}

    bool GetClamp() const { return mClamp; }
    ETevOp GetOp() const { return mOp; }
    ETevBias GetBias() const { return mBias; }
    ETevScale GetScale() const { return mScale; }
    ETevOutput GetOutput() const { return mOutput; }

  private:
    bool mClamp;
    ETevOp mOp;
    ETevBias mBias;
    ETevScale mScale;
    ETevOutput mOutput;
  };

  class CTevPass {
  public:
    CTevPass(const ColorPass& colorPass, const AlphaPass& alphaPass,
             const CTevOp& colorOp = CTevOp(), const CTevOp& alphaOp = CTevOp())
    : mId(sNextUniquePass++)
    , mColorPass(colorPass)
    , mAlphaPass(alphaPass)
    , mColorOp(colorOp)
    , mAlphaOp(alphaOp) {}

    void Execute(int) const;

  private:
    uint mId;
    ColorPass mColorPass;
    AlphaPass mAlphaPass;
    CTevOp mColorOp;
    CTevOp mAlphaOp;
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
