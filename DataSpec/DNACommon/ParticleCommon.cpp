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
        m_elem.reset(new struct REKeyframeEmitterP);
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
        m_elem.reset(new struct REKeyframeEmitter);
        break;
    case SBIG('IRND'):
        m_elem.reset(new struct REInitialRandom);
        break;
    case SBIG('KEYP'):
        m_elem.reset(new struct REKeyframeEmitterP);
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
        m_elem.reset(new struct IEKeyframeEmitterP);
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
    case SBIG('KEYP'):
        m_elem.reset(new struct IEKeyframeEmitterP);
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

}
}
