#ifndef __URDE_CGUISTATICIMAGE_HPP__
#define __URDE_CGUISTATICIMAGE_HPP__

#include "CGuiPane.hpp"
#include "CToken.hpp"

namespace urde
{
class CTexture;

class CGuiStaticImage : public CGuiPane
{
public:
    enum class EMaterialType
    {
        OneTexture = 0,
        TwoTextures = 1
    };
private:
    EMaterialType x114_materialType;
    TLockedToken<CTexture> x118_texture1;
    TLockedToken<CTexture> x11c_texture2;
    TResId x120_textureID1;
    TResId x124_textureID2;
    EGuiTextureClampModeHorz x128_clampH;
    EGuiTextureClampModeVert x12c_clampV;
    std::vector<float> x130_clampedUVs;
    std::vector<float> x140_UVs;
public:
    CGuiStaticImage(const CGuiWidgetParms& parms, float xDim, float zDim,
                    const zeus::CVector3f& scaleCenter,
                    EGuiTextureClampModeHorz clampH, EGuiTextureClampModeVert clampV,
                    CGuiStaticImage::EMaterialType matType, TResId txtrId1, TResId txtrId2,
                    const std::vector<float>& frame, bool useTexture);
    FourCC GetWidgetTypeID() const {return FOURCC('IMAG');}

    void ScaleDimensions(const zeus::CVector3f& scale);
    void Draw(const CGuiWidgetDrawParms& parms) const;
    std::vector<TResId> GetTextureAssets() const;

    static CGuiStaticImage* Create(CGuiFrame* frame, CInputStream& in, bool flag);
};

}

#endif // __URDE_CGUISTATICIMAGE_HPP__
