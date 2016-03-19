#ifndef __URDE_CINSTRUCTION_HPP__
#define __URDE_CINSTRUCTION_HPP__

#include "CToken.hpp"
#include "CGuiTextSupport.hpp"
#include <vector>

namespace urde
{
class CFontRenderState;
class CTextRenderBuffer;

class CInstruction
{
public:
    virtual ~CInstruction() = default;
    virtual void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const=0;
    virtual void GetAssets(std::vector<CToken>& assetsOut) const;
    virtual size_t GetAssetCount() const;
};

class CColorInstruction : public CInstruction
{
    EColorType x4_cType;
    CTextColor x8_color;
public:
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CColorOverrideInstruction : public CInstruction
{
    int x4_overrideIdx;
    CTextColor x8_color;
public:
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CFontInstruction : public CInstruction
{
};

class CExtraLineSpaceInstruction : public CInstruction
{
};

class CLineInstruction : public CInstruction
{
};

class CLineSpacingInstruction : public CInstruction
{
};

class CPopStateInstruction : public CInstruction
{
};

class CPushStateInstruction : public CInstruction
{
};

class CRemoveColorOverrideInstruction : public CInstruction
{
};

class CImageInstruction : public CInstruction
{
};

class CTextInstruction : public CInstruction
{
};

class CBlockInstruction : public CInstruction
{
};

class CWordInstruction : public CInstruction
{
};

}

#endif // __URDE_CINSTRUCTION_HPP__
