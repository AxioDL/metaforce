#ifndef __PSHAG_CMODEL_HPP__
#define __PSHAG_CMODEL_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"
#include "CFactoryMgr.hpp"
#include "CToken.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class IObjectStore;
class CTexture;

struct CModelFlags
{
    u8 f1; /* Blend state 3/5 enable additive */
    u8 f2;
    u16 f3; /* Depth state */
    zeus::CColor color; /* Set into kcolor slot specified by material */

    /* depth flags
        0x8: greater
        0x10: non-inclusive
     */
};

class CBooModel
{
public:
    struct CSurface
    {
        const u8* m_data;
        CBooModel* m_parent = nullptr;
        CSurface* m_next = nullptr;
    };
private:
    std::vector<CSurface>* x0_surfaces;
    const u8* x4_matSet;
    const void* x8_vbo;
    const void* xc_ibo;
    std::vector<TLockedToken<CTexture>>* x1c_textures;
    zeus::CAABox x20_aabb;
    CSurface* x38_firstUnsortedSurface = nullptr;
    CSurface* x3c_firstSortedSurface = nullptr;
    bool x40_24_ : 1;
    bool x40_25_ : 1;
    u8 x41_shortNormals;
public:
    CBooModel(std::vector<CSurface>* surfaces, std::vector<TLockedToken<CTexture>>* textures,
              const u8* matSet, const void* vbo, const void* ibo, const zeus::CAABox& aabb,
              u8 shortNormals, bool unk);

    static void MakeTexuresFromMats(const u8* dataIn,
                                    std::vector<TLockedToken<CTexture>>& toksOut,
                                    IObjectStore& store);

    const u8* GetMaterialByIndex(int idx) const;
};

class CModel
{
public:
    struct SShader
    {
        std::vector<TLockedToken<CTexture>> x0_textures;
        const u8* x10_data;
    };
private:
    std::unique_ptr<u8[]> x0_data;
    u32 x4_dataLen;
    std::vector<CBooModel::CSurface> x8_surfaces;
    std::vector<SShader> x18_matSets;
    std::unique_ptr<CBooModel> x28_modelInst;
    CModel* x30_next = nullptr;
    CModel* x34_prev = nullptr;
public:
    CModel(std::unique_ptr<u8[]>&& in, u32 dataLen, IObjectStore* store);
    void Draw(const CModelFlags& flags) const;
    void Touch(int) const;
    bool IsLoaded(int) const;
};

CFactoryFnReturn FModelFactory(const urde::SObjectTag& tag,
                               std::unique_ptr<u8[]>&& in, u32 len,
                               const urde::CVParamTransfer& vparms);

}

#endif // __PSHAG_CMODEL_HPP__
