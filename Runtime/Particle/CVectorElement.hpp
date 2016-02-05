#ifndef __RETRO_CVECTORELEMENT_HPP__
#define __RETRO_CVECTORELEMENT_HPP__

#include "IElement.hpp"
#include "CVector3f.hpp"

namespace Retro
{

class CVECone : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVETimeChain : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEAngleCone : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEAdd : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVECircleCluster : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEConstant : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEFastConstant : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
    bool IsFastConstant() const {return true;}
};

class CVECircle : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEKeyframeEmitter : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEMultiply : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVERealToVector : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEPulse : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEParticleVelocity : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVESPOS : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEPLCO : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEPLOC : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEPSOR : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEPSOF : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

}

#endif // __RETRO_CVECTORELEMENT_HPP__
