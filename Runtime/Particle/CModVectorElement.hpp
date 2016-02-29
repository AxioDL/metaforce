#ifndef __PSHAG_CMODVECTORELEMENT_HPP__
#define __PSHAG_CMODVECTORELEMENT_HPP__

#include "IElement.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Mod_Vector_Elements */

namespace pshag
{

class CMVEImplosion : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_implPoint;
    std::unique_ptr<CRealElement> x8_magScale;
    std::unique_ptr<CRealElement> xc_maxMag;
    std::unique_ptr<CRealElement> x10_minMag;
    bool x14_enableMinMag;
public:
    CMVEImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d, bool e)
    : x4_implPoint(a), x8_magScale(b), xc_maxMag(c), x10_minMag(d), x14_enableMinMag(e) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVEExponentialImplosion : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_implPoint;
    std::unique_ptr<CRealElement> x8_magScale;
    std::unique_ptr<CRealElement> xc_maxMag;
    std::unique_ptr<CRealElement> x10_minMag;
    bool x14_enableMinMag;
public:
    CMVEExponentialImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d, bool e)
    : x4_implPoint(a), x8_magScale(b), xc_maxMag(c), x10_minMag(d), x14_enableMinMag(e) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVELinearImplosion : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_implPoint;
    std::unique_ptr<CRealElement> x8_magScale;
    std::unique_ptr<CRealElement> xc_maxMag;
    std::unique_ptr<CRealElement> x10_minMag;
    bool x14_enableMinMag;
public:
    CMVELinearImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d, bool e)
    : x4_implPoint(a), x8_magScale(b), xc_maxMag(c), x10_minMag(d), x14_enableMinMag(e) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVETimeChain : public CModVectorElement
{
    std::unique_ptr<CModVectorElement> x4_a;
    std::unique_ptr<CModVectorElement> x8_b;
    std::unique_ptr<CIntElement> xc_swFrame;
public:
    CMVETimeChain(CModVectorElement* a, CModVectorElement* b, CIntElement* c)
    : x4_a(a), x8_b(b), xc_swFrame(c) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVEBounce : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_planePoint;
    std::unique_ptr<CVectorElement> x8_planeNormal;
    std::unique_ptr<CRealElement> xc_friction;
    std::unique_ptr<CRealElement> x10_restitution;
    bool x14_planePrecomputed;
    bool x15_dieOnPenetrate;
    Zeus::CVector3f x18_planeValidatedNormal;
    float x24_planeD;
public:
    CMVEBounce(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d, bool e);
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVEConstant : public CModVectorElement
{
    std::unique_ptr<CRealElement> x4_x;
    std::unique_ptr<CRealElement> x8_y;
    std::unique_ptr<CRealElement> xc_z;
public:
    CMVEConstant(CRealElement* a, CRealElement* b, CRealElement* c)
    : x4_x(a), x8_y(b), xc_z(c) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVEFastConstant : public CModVectorElement
{
    Zeus::CVector3f x4_val;
public:
    CMVEFastConstant(float a, float b, float c)
    : x4_val(a, b, c) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVEGravity : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
public:
    CMVEGravity(CVectorElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVEExplode : public CModVectorElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
public:
    CMVEExplode(CRealElement* a, CRealElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVESetPosition : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
public:
    CMVESetPosition(CVectorElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVEPulse : public CModVectorElement
{
    std::unique_ptr<CIntElement> x4_aDuration;
    std::unique_ptr<CIntElement> x8_bDuration;
    std::unique_ptr<CModVectorElement> xc_aVal;
    std::unique_ptr<CModVectorElement> x10_bVal;
public:
    CMVEPulse(CIntElement* a, CIntElement* b, CModVectorElement* c, CModVectorElement* d)
    : x4_aDuration(a), x8_bDuration(b), xc_aVal(c), x10_bVal(d) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVEWind : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_velocity;
    std::unique_ptr<CRealElement> x8_factor;
public:
    CMVEWind(CVectorElement* a, CRealElement* b)
    : x4_velocity(a), x8_factor(b) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

class CMVESwirl : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_helixPoint;
    std::unique_ptr<CVectorElement> x8_curveBinormal;
    std::unique_ptr<CRealElement> xc_targetRadius;
    std::unique_ptr<CRealElement> x10_tangentialVelocity;
public:
    CMVESwirl(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d)
    : x4_helixPoint(a), x8_curveBinormal(b), xc_targetRadius(c), x10_tangentialVelocity(d) {}
    bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const;
};

}

#endif // __PSHAG_CMODVECTORELEMENT_HPP__
