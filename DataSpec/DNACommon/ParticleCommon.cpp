#include "ParticleCommon.hpp"

namespace Retro
{
namespace DNAParticle
{
LogVisor::LogModule LogModule("Retro::DNAParticle");

void RealElementFactory::read(Athena::io::YAMLDocReader& r)
{
    const auto& mapChildren = r.getCurNode()->m_mapChildren;
    if (mapChildren.empty())
    {
        m_elem.reset();
        return;
    }

    const auto& elem = mapChildren[0];
    if (elem.first.size() < 4)
        LogModule.report(LogVisor::FatalError, "short FourCC in element '%s'", elem.first.c_str());

    switch (*reinterpret_cast<const uint32_t*>(elem.first.data()))
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
    case SBIG('ISWT'):
        m_elem.reset(new struct REISWT);
        break;
    case SBIG('CLTN'):
        m_elem.reset(new struct RECompareLessThan);
        break;
    case SBIG('CEQL'):
        m_elem.reset(new struct RECompareEquals);
        break;
    case SBIG('PAP1'):
        m_elem.reset(new struct REParticleAccessParam1);
        break;
    case SBIG('PAP2'):
        m_elem.reset(new struct REParticleAccessParam2);
        break;
    case SBIG('PAP3'):
        m_elem.reset(new struct REParticleAccessParam3);
        break;
    case SBIG('PAP4'):
        m_elem.reset(new struct REParticleAccessParam4);
        break;
    case SBIG('PAP5'):
        m_elem.reset(new struct REParticleAccessParam5);
        break;
    case SBIG('PAP6'):
        m_elem.reset(new struct REParticleAccessParam6);
        break;
    case SBIG('PAP7'):
        m_elem.reset(new struct REParticleAccessParam7);
        break;
    case SBIG('PAP8'):
        m_elem.reset(new struct REParticleAccessParam8);
        break;
    case SBIG('PSLL'):
        m_elem.reset(new struct REPSLL);
        break;
    case SBIG('PRLW'):
        m_elem.reset(new struct REPRLW);
        break;
    case SBIG('SUB_'):
        m_elem.reset(new struct RESubtract);
        break;
    case SBIG('VMAG'):
        m_elem.reset(new struct REVectorMagnitude);
        break;
    case SBIG('VXTR'):
        m_elem.reset(new struct REVectorXToReal);
        break;
    case SBIG('VYTR'):
        m_elem.reset(new struct REVectorYToReal);
        break;
    case SBIG('VZTR'):
        m_elem.reset(new struct REVectorZToReal);
        break;
    case SBIG('CEXT'):
        m_elem.reset(new struct RECEXT);
        break;
    case SBIG('ITRL'):
        m_elem.reset(new struct REIntTimesReal);
        break;
    default:
        m_elem.reset();
        return;
    }
    r.enterSubRecord(elem.first.c_str());
    m_elem->read(r);
    r.leaveSubRecord();
}

void RealElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassID());
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
    case SBIG('ISWT'):
        m_elem.reset(new struct REISWT);
        break;
    case SBIG('CLTN'):
        m_elem.reset(new struct RECompareLessThan);
        break;
    case SBIG('CEQL'):
        m_elem.reset(new struct RECompareEquals);
        break;
    case SBIG('PAP1'):
        m_elem.reset(new struct REParticleAccessParam1);
        break;
    case SBIG('PAP2'):
        m_elem.reset(new struct REParticleAccessParam2);
        break;
    case SBIG('PAP3'):
        m_elem.reset(new struct REParticleAccessParam3);
        break;
    case SBIG('PAP4'):
        m_elem.reset(new struct REParticleAccessParam4);
        break;
    case SBIG('PAP5'):
        m_elem.reset(new struct REParticleAccessParam5);
        break;
    case SBIG('PAP6'):
        m_elem.reset(new struct REParticleAccessParam6);
        break;
    case SBIG('PAP7'):
        m_elem.reset(new struct REParticleAccessParam7);
        break;
    case SBIG('PAP8'):
        m_elem.reset(new struct REParticleAccessParam8);
        break;
    case SBIG('PSLL'):
        m_elem.reset(new struct REPSLL);
        break;
    case SBIG('PRLW'):
        m_elem.reset(new struct REPRLW);
        break;
    case SBIG('SUB_'):
        m_elem.reset(new struct RESubtract);
        break;
    case SBIG('VMAG'):
        m_elem.reset(new struct REVectorMagnitude);
        break;
    case SBIG('VXTR'):
        m_elem.reset(new struct REVectorXToReal);
        break;
    case SBIG('VYTR'):
        m_elem.reset(new struct REVectorYToReal);
        break;
    case SBIG('VZTR'):
        m_elem.reset(new struct REVectorZToReal);
        break;
    case SBIG('CEXT'):
        m_elem.reset(new struct RECEXT);
        break;
    case SBIG('ITRL'):
        m_elem.reset(new struct REIntTimesReal);
        break;
    case SBIG('NONE'):
        m_elem.reset();
        return;
    default:
        m_elem.reset();
        LogModule.report(LogVisor::FatalError, "Unknown RealElement class %.4s @%" PRIi64, &clsId, r.position());
        return;
    }
    m_elem->read(r);
}

void RealElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassID(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}


void IntElementFactory::read(Athena::io::YAMLDocReader& r)
{
    const auto& mapChildren = r.getCurNode()->m_mapChildren;
    if (mapChildren.empty())
    {
        m_elem.reset();
        return;
    }

    const auto& elem = mapChildren[0];
    if (elem.first.size() < 4)
        LogModule.report(LogVisor::FatalError, "short FourCC in element '%s'", elem.first.c_str());

    switch (*reinterpret_cast<const uint32_t*>(elem.first.data()))
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
    case SBIG('GTCP'):
        m_elem.reset(new struct IEGTCP);
        break;
    case SBIG('MODU'):
        m_elem.reset(new struct IEModulo);
        break;
    case SBIG('SUB_'):
        m_elem.reset(new struct IESubtract);
        break;
    default:
        m_elem.reset();
        return;
    }
    r.enterSubRecord(elem.first.c_str());
    m_elem->read(r);
    r.leaveSubRecord();
}

void IntElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassID());
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
    case SBIG('GTCP'):
        m_elem.reset(new struct IEGTCP);
        break;
    case SBIG('MODU'):
        m_elem.reset(new struct IEModulo);
        break;
    case SBIG('SUB_'):
        m_elem.reset(new struct IESubtract);
        break;
    case SBIG('NONE'):
        m_elem.reset();
        return;
    default:
        m_elem.reset();
        LogModule.report(LogVisor::FatalError, "Unknown IntElement class %.4s @%" PRIi64, &clsId, r.position());
        return;
    }
    m_elem->read(r);
}

void IntElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassID(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}

void VectorElementFactory::read(Athena::io::YAMLDocReader& r)
{
    const auto& mapChildren = r.getCurNode()->m_mapChildren;
    if (mapChildren.empty())
    {
        m_elem.reset();
        return;
    }

    const auto& elem = mapChildren[0];
    if (elem.first.size() < 4)
        LogModule.report(LogVisor::FatalError, "short FourCC in element '%s'", elem.first.c_str());

    switch (*reinterpret_cast<const uint32_t*>(elem.first.data()))
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
    case SBIG('PVEL'):
        m_elem.reset(new struct VEParticleVelocity);
        break;
    case SBIG('SPOS'):
        m_elem.reset(new struct VESPOS);
        break;
    case SBIG('PLCO'):
        m_elem.reset(new struct VEPLCO);
        break;
    case SBIG('PLOC'):
        m_elem.reset(new struct VEPLOC);
        break;
    case SBIG('PSOR'):
        m_elem.reset(new struct VEPSOR);
        break;
    case SBIG('PSOF'):
        m_elem.reset(new struct VEPSOF);
        break;
    default:
        m_elem.reset();
        return;
    }
    r.enterSubRecord(elem.first.c_str());
    m_elem->read(r);
    r.leaveSubRecord();
}

void VectorElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassID());
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
    case SBIG('PVEL'):
        m_elem.reset(new struct VEParticleVelocity);
        break;
    case SBIG('SPOS'):
        m_elem.reset(new struct VESPOS);
        break;
    case SBIG('PLCO'):
        m_elem.reset(new struct VEPLCO);
        break;
    case SBIG('PLOC'):
        m_elem.reset(new struct VEPLOC);
        break;
    case SBIG('PSOR'):
        m_elem.reset(new struct VEPSOR);
        break;
    case SBIG('PSOF'):
        m_elem.reset(new struct VEPSOF);
        break;
    case SBIG('NONE'):
        m_elem.reset();
        return;
    default:
        m_elem.reset();
        LogModule.report(LogVisor::FatalError, "Unknown VectorElement class %.4s @%" PRIi64, &clsId, r.position());
        return;
    }
    m_elem->read(r);
}

void VectorElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassID(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}


void ColorElementFactory::read(Athena::io::YAMLDocReader& r)
{
    const auto& mapChildren = r.getCurNode()->m_mapChildren;
    if (mapChildren.empty())
    {
        m_elem.reset();
        return;
    }

    const auto& elem = mapChildren[0];
    if (elem.first.size() < 4)
        LogModule.report(LogVisor::FatalError, "short FourCC in element '%s'", elem.first.c_str());

    switch (*reinterpret_cast<const uint32_t*>(elem.first.data()))
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
    r.enterSubRecord(elem.first.c_str());
    m_elem->read(r);
    r.leaveSubRecord();
}

void ColorElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassID());
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
    case SBIG('NONE'):
        m_elem.reset();
        return;
    default:
        m_elem.reset();
        LogModule.report(LogVisor::FatalError, "Unknown ColorElement class %.4s @%" PRIi64, &clsId, r.position());
        return;
    }
    m_elem->read(r);
}

void ColorElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassID(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}


void ModVectorElementFactory::read(Athena::io::YAMLDocReader& r)
{
    const auto& mapChildren = r.getCurNode()->m_mapChildren;
    if (mapChildren.empty())
    {
        m_elem.reset();
        return;
    }

    const auto& elem = mapChildren[0];
    if (elem.first.size() < 4)
        LogModule.report(LogVisor::FatalError, "short FourCC in element '%s'", elem.first.c_str());

    switch (*reinterpret_cast<const uint32_t*>(elem.first.data()))
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
    r.enterSubRecord(elem.first.c_str());
    m_elem->read(r);
    r.leaveSubRecord();
}

void ModVectorElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassID());
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
    case SBIG('NONE'):
        m_elem.reset();
        return;
    default:
        m_elem.reset();
        LogModule.report(LogVisor::FatalError, "Unknown ModVectorElement class %.4s @%" PRIi64, &clsId, r.position());
        return;
    }
    m_elem->read(r);
}

void ModVectorElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassID(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}


void EmitterElementFactory::read(Athena::io::YAMLDocReader& r)
{
    const auto& mapChildren = r.getCurNode()->m_mapChildren;
    if (mapChildren.empty())
    {
        m_elem.reset();
        return;
    }

    const auto& elem = mapChildren[0];
    if (elem.first.size() < 4)
        LogModule.report(LogVisor::FatalError, "short FourCC in element '%s'", elem.first.c_str());

    switch (*reinterpret_cast<const uint32_t*>(elem.first.data()))
    {
    case SBIG('SETR'):
        m_elem.reset(new struct EESimpleEmitterTR);
        break;
    case SBIG('SEMR'):
        m_elem.reset(new struct EESimpleEmitter);
        break;
    case SBIG('SPHE'):
        m_elem.reset(new struct VESphere);
        break;
    case SBIG('ASPH'):
        m_elem.reset(new struct VEAngleSphere);
        break;
    default:
        m_elem.reset();
        return;
    }
    r.enterSubRecord(elem.first.c_str());
    m_elem->read(r);
    r.leaveSubRecord();
}

void EmitterElementFactory::write(Athena::io::YAMLDocWriter& w) const
{
    if (m_elem)
    {
        w.enterSubRecord(m_elem->ClassID());
        m_elem->write(w);
        w.leaveSubRecord();
    }
}

size_t EmitterElementFactory::binarySize(size_t __isz) const
{
    if (m_elem)
        return m_elem->binarySize(__isz + 4);
    else
        return __isz + 4;
}

void EmitterElementFactory::read(Athena::io::IStreamReader& r)
{
    uint32_t clsId;
    r.readBytesToBuf(&clsId, 4);
    switch (clsId)
    {
    case SBIG('SETR'):
        m_elem.reset(new struct EESimpleEmitterTR);
        break;
    case SBIG('SEMR'):
        m_elem.reset(new struct EESimpleEmitter);
        break;
    case SBIG('SPHE'):
        m_elem.reset(new struct VESphere);
        break;
    case SBIG('ASPH'):
        m_elem.reset(new struct VEAngleSphere);
        break;
    case SBIG('NONE'):
        m_elem.reset();
        return;
    default:
        m_elem.reset();
        LogModule.report(LogVisor::FatalError, "Unknown EmitterElement class %.4s @%" PRIi64, &clsId, r.position());
        return;
    }
    m_elem->read(r);
}

void EmitterElementFactory::write(Athena::io::IStreamWriter& w) const
{
    if (m_elem)
    {
        w.writeBytes((atInt8*)m_elem->ClassID(), 4);
        m_elem->write(w);
    }
    else
        w.writeBytes((atInt8*)"NONE", 4);
}

}
}
