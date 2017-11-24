#ifndef __URDE_CSIMPLESHADOW_HPP__
#define __URDE_CSIMPLESHADOW_HPP__

namespace urde
{
class CTexture;

class CSimpleShadow
{
public:
    CSimpleShadow() = default;
    CSimpleShadow(float, float, float, float);
    void Render(const CTexture* tex) const;
};
}

#endif // __URDE_CSIMPLESHADOW_HPP__
