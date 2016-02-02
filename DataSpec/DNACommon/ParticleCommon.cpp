#include "ParticleCommon.hpp"

namespace Retro
{
namespace DNAParticle
{

void RealElementFactory::read(Athena::io::YAMLDocReader& r)
{
    if (r.enterSubRecord("LFTW"))
    {
        m_elem.reset(new struct RELifetimeTween);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CNST"))
    {
        m_elem.reset(new struct REConstant);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CHAN"))
    {
        m_elem.reset(new struct RETimeChain);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("ADD_"))
    {
        m_elem.reset(new struct REAdd);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CLMP"))
    {
        m_elem.reset(new struct REClamp);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("KEYE"))
    {
        m_elem.reset(new struct REKeyframeEmitter);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("IRND"))
    {
        m_elem.reset(new struct REInitialRandom);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("KEYP"))
    {
        m_elem.reset(new struct REKeyframeEmitter);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("RAND"))
    {
        m_elem.reset(new struct RERandom);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("MULT"))
    {
        m_elem.reset(new struct REMultiply);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("PULS"))
    {
        m_elem.reset(new struct REPulse);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("SCAL"))
    {
        m_elem.reset(new struct RETimeScale);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("RLPT"))
    {
        m_elem.reset(new struct RELifetimePercent);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("SINE"))
    {
        m_elem.reset(new struct RESineWave);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else
        m_elem.reset();
}

void RealElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassName());
        m_elem->write(w);
        w.leaveSubRecord();
    }
}

size_t RealElementFactory::binarySize(size_t __isz) const
{
    if (m_elem)
        return m_elem->binarySize(__isz + 4);
    else
        return __isz + 4;
}

void RealElementFactory::read(Athena::io::IStreamReader& r)
{
    uint32_t clsId;
    r.readBytesToBuf(&clsId, 4);
    switch (clsId)
    {
    case SBIG('LFTW'):
        m_elem.reset(new struct RELifetimeTween);
        break;
    case SBIG('CNST'):
        m_elem.reset(new struct REConstant);
        break;
    case SBIG('CHAN'):
        m_elem.reset(new struct RETimeChain);
        break;
    case SBIG('ADD_'):
        m_elem.reset(new struct REAdd);
        break;
    case SBIG('CLMP'):
        m_elem.reset(new struct REClamp);
        break;
    case SBIG('KEYE'):
    case SBIG('KEYP'):
        m_elem.reset(new struct REKeyframeEmitter);
        break;
    case SBIG('IRND'):
        m_elem.reset(new struct REInitialRandom);
        break;
    case SBIG('RAND'):
        m_elem.reset(new struct RERandom);
        break;
    case SBIG('MULT'):
        m_elem.reset(new struct REMultiply);
        break;
    case SBIG('PULS'):
        m_elem.reset(new struct REPulse);
        break;
    case SBIG('SCAL'):
        m_elem.reset(new struct RETimeScale);
        break;
    case SBIG('RLPT'):
        m_elem.reset(new struct RELifetimePercent);
        break;
    case SBIG('SINE'):
        m_elem.reset(new struct RESineWave);
        break;
    default:
        m_elem.reset();
        return;
    }
    m_elem->read(r);
}

void RealElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassName(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}


void IntElementFactory::read(Athena::io::YAMLDocReader& r)
{
    if (r.enterSubRecord("KEYE"))
    {
        m_elem.reset(new struct IEKeyframeEmitter);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("DETH"))
    {
        m_elem.reset(new struct IEDeath);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CLMP"))
    {
        m_elem.reset(new struct IEClamp);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CHAN"))
    {
        m_elem.reset(new struct IETimeChain);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("ADD_"))
    {
        m_elem.reset(new struct IEAdd);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CNST"))
    {
        m_elem.reset(new struct IEConstant);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("IMPL"))
    {
        m_elem.reset(new struct IEImpulse);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("ILPT"))
    {
        m_elem.reset(new struct IELifetimePercent);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("IRND"))
    {
        m_elem.reset(new struct IEInitialRandom);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("PULS"))
    {
        m_elem.reset(new struct IEPulse);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("MULT"))
    {
        m_elem.reset(new struct IEMultiply);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("KEYP"))
    {
        m_elem.reset(new struct IEKeyframeEmitter);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("SPAH"))
    {
        m_elem.reset(new struct IESampleAndHold);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("RAND"))
    {
        m_elem.reset(new struct IERandom);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("TSCL"))
    {
        m_elem.reset(new struct IETimeScale);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else
        m_elem.reset();
}

void IntElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassName());
        m_elem->write(w);
        w.leaveSubRecord();
    }
}

size_t IntElementFactory::binarySize(size_t __isz) const
{
    if (m_elem)
        return m_elem->binarySize(__isz + 4);
    else
        return __isz + 4;
}

void IntElementFactory::read(Athena::io::IStreamReader& r)
{
    uint32_t clsId;
    r.readBytesToBuf(&clsId, 4);
    switch (clsId)
    {
    case SBIG('KEYE'):
    case SBIG('KEYP'):
        m_elem.reset(new struct IEKeyframeEmitter);
        break;
    case SBIG('DETH'):
        m_elem.reset(new struct IEDeath);
        break;
    case SBIG('CLMP'):
        m_elem.reset(new struct IEClamp);
        break;
    case SBIG('CHAN'):
        m_elem.reset(new struct IETimeChain);
        break;
    case SBIG('ADD_'):
        m_elem.reset(new struct IEAdd);
        break;
    case SBIG('CNST'):
        m_elem.reset(new struct IEConstant);
        break;
    case SBIG('IMPL'):
        m_elem.reset(new struct IEImpulse);
        break;
    case SBIG('ILPT'):
        m_elem.reset(new struct IELifetimePercent);
        break;
    case SBIG('IRND'):
        m_elem.reset(new struct IEInitialRandom);
        break;
    case SBIG('PULS'):
        m_elem.reset(new struct IEPulse);
        break;
    case SBIG('MULT'):
        m_elem.reset(new struct IEMultiply);
        break;
    case SBIG('SPAH'):
        m_elem.reset(new struct IESampleAndHold);
        break;
    case SBIG('RAND'):
        m_elem.reset(new struct IERandom);
        break;
    case SBIG('TSCL'):
        m_elem.reset(new struct IETimeScale);
        break;
    default:
        m_elem.reset();
        return;
    }
    m_elem->read(r);
}

void IntElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassName(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}

void VectorElementFactory::read(Athena::io::YAMLDocReader& r)
{
    if (r.enterSubRecord("CONE"))
    {
        m_elem.reset(new struct VECone);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CHAN"))
    {
        m_elem.reset(new struct VETimeChain);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("ANGC"))
    {
        m_elem.reset(new struct VEAngleCone);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("ADD_"))
    {
        m_elem.reset(new struct VEAdd);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CCLU"))
    {
        m_elem.reset(new struct VECircleCluster);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CNST"))
    {
        m_elem.reset(new struct VEConstant);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CIRC"))
    {
        m_elem.reset(new struct VECircle);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("KEYE"))
    {
        m_elem.reset(new struct VEKeyframeEmitter);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("KEYP"))
    {
        m_elem.reset(new struct VEKeyframeEmitter);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("MULT"))
    {
        m_elem.reset(new struct VEMultiply);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("RTOV"))
    {
        m_elem.reset(new struct VERealToVector);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("PULS"))
    {
        m_elem.reset(new struct VEPulse);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else
        m_elem.reset();
}

void VectorElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassName());
        m_elem->write(w);
        w.leaveSubRecord();
    }
}

size_t VectorElementFactory::binarySize(size_t __isz) const
{
    if (m_elem)
        return m_elem->binarySize(__isz + 4);
    else
        return __isz + 4;
}

void VectorElementFactory::read(Athena::io::IStreamReader& r)
{
    uint32_t clsId;
    r.readBytesToBuf(&clsId, 4);
    switch (clsId)
    {
    case SBIG('CONE'):
        m_elem.reset(new struct VECone);
        break;
    case SBIG('CHAN'):
        m_elem.reset(new struct VETimeChain);
        break;
    case SBIG('ANGC'):
        m_elem.reset(new struct VEAngleCone);
        break;
    case SBIG('ADD_'):
        m_elem.reset(new struct VEAdd);
        break;
    case SBIG('CCLU'):
        m_elem.reset(new struct VECircleCluster);
        break;
    case SBIG('CNST'):
        m_elem.reset(new struct VEConstant);
        break;
    case SBIG('CIRC'):
        m_elem.reset(new struct VECircle);
        break;
    case SBIG('KEYE'):
    case SBIG('KEYP'):
        m_elem.reset(new struct VEKeyframeEmitter);
        break;
    case SBIG('MULT'):
        m_elem.reset(new struct VEMultiply);
        break;
    case SBIG('RTOV'):
        m_elem.reset(new struct VERealToVector);
        break;
    case SBIG('PULS'):
        m_elem.reset(new struct VEPulse);
        break;
    default:
        m_elem.reset();
        return;
    }
    m_elem->read(r);
}

void VectorElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassName(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}


void ColorElementFactory::read(Athena::io::YAMLDocReader& r)
{
    if (r.enterSubRecord("KEYE"))
    {
        m_elem.reset(new struct CEKeyframeEmitter);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CNST"))
    {
        m_elem.reset(new struct CEConstant);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CHAN"))
    {
        m_elem.reset(new struct CETimeChain);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CFDE"))
    {
        m_elem.reset(new struct CEFadeEnd);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("FADE"))
    {
        m_elem.reset(new struct CEFade);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("KEYP"))
    {
        m_elem.reset(new struct CEKeyframeEmitter);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("PULS"))
    {
        m_elem.reset(new struct CEPulse);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else
        m_elem.reset();
}

void ColorElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassName());
        m_elem->write(w);
        w.leaveSubRecord();
    }
}

size_t ColorElementFactory::binarySize(size_t __isz) const
{
    if (m_elem)
        return m_elem->binarySize(__isz + 4);
    else
        return __isz + 4;
}

void ColorElementFactory::read(Athena::io::IStreamReader& r)
{
    uint32_t clsId;
    r.readBytesToBuf(&clsId, 4);
    switch (clsId)
    {
    case SBIG('KEYE'):
    case SBIG('KEYP'):
        m_elem.reset(new struct CEKeyframeEmitter);
        break;
    case SBIG('CNST'):
        m_elem.reset(new struct CEConstant);
        break;
    case SBIG('CHAN'):
        m_elem.reset(new struct CETimeChain);
        break;
    case SBIG('CFDE'):
        m_elem.reset(new struct CEFadeEnd);
        break;
    case SBIG('FADE'):
        m_elem.reset(new struct CEFade);
        break;
    case SBIG('PULS'):
        m_elem.reset(new struct CEPulse);
        break;
    default:
        m_elem.reset();
        return;
    }
    m_elem->read(r);
}

void ColorElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassName(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}


void ModVectorElementFactory::read(Athena::io::YAMLDocReader& r)
{
    if (r.enterSubRecord("IMPL"))
    {
        m_elem.reset(new struct MVEImplosion);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("EMPL"))
    {
        m_elem.reset(new struct MVEExponentialImplosion);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CHAN"))
    {
        m_elem.reset(new struct MVETimeChain);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("BNCE"))
    {
        m_elem.reset(new struct MVEBounce);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("CNST"))
    {
        m_elem.reset(new struct MVEConstant);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("GRAV"))
    {
        m_elem.reset(new struct MVEGravity);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("EXPL"))
    {
        m_elem.reset(new struct MVEExplode);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("SPOS"))
    {
        m_elem.reset(new struct MVESetPosition);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("LMPL"))
    {
        m_elem.reset(new struct MVELinearImplosion);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("PULS"))
    {
        m_elem.reset(new struct MVEPulse);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("WIND"))
    {
        m_elem.reset(new struct MVEWind);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else if (r.enterSubRecord("SWRL"))
    {
        m_elem.reset(new struct MVESwirl);
        m_elem->read(r);
        r.leaveSubRecord();
    }
    else
        m_elem.reset();
}

void ModVectorElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassName());
        m_elem->write(w);
        w.leaveSubRecord();
    }
}

size_t ModVectorElementFactory::binarySize(size_t __isz) const
{
    if (m_elem)
        return m_elem->binarySize(__isz + 4);
    else
        return __isz + 4;
}

void ModVectorElementFactory::read(Athena::io::IStreamReader& r)
{
    uint32_t clsId;
    r.readBytesToBuf(&clsId, 4);
    switch (clsId)
    {
    case SBIG('IMPL'):
        m_elem.reset(new struct MVEImplosion);
        break;
    case SBIG('EMPL'):
        m_elem.reset(new struct MVEExponentialImplosion);
        break;
    case SBIG('CHAN'):
        m_elem.reset(new struct MVETimeChain);
        break;
    case SBIG('BNCE'):
        m_elem.reset(new struct MVEBounce);
        break;
    case SBIG('CNST'):
        m_elem.reset(new struct MVEConstant);
        break;
    case SBIG('GRAV'):
        m_elem.reset(new struct MVEGravity);
        break;
    case SBIG('EXPL'):
        m_elem.reset(new struct MVEExplode);
        break;
    case SBIG('SPOS'):
        m_elem.reset(new struct MVESetPosition);
        break;
    case SBIG('LMPL'):
        m_elem.reset(new struct MVELinearImplosion);
        break;
    case SBIG('PULS'):
        m_elem.reset(new struct MVEPulse);
        break;
    case SBIG('WIND'):
        m_elem.reset(new struct MVEWind);
        break;
    case SBIG('SWRL'):
        m_elem.reset(new struct MVESwirl);
        break;
    default:
        m_elem.reset();
        return;
    }
    m_elem->read(r);
}

void ModVectorElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassName(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}

}
}
