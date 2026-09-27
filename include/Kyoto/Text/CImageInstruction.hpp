#ifndef _CIMAGEINSTRUCTION
#define _CIMAGEINSTRUCTION

#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/Text/CInstruction.hpp"

class CImageInstruction : public CInstruction {
public:
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void GetAssets(rstl::vector< CToken >& assetsOut) const override;
  uint GetAssetCount() const override { return mImage.GetImages().size(); }

  explicit CImageInstruction(const CFontImageDef& image) : mImage(image) {}

private:
  CFontImageDef mImage;
};

CHECK_SIZEOF(CImageInstruction, 0x20)

#endif // _CIMAGEINSTRUCTION
