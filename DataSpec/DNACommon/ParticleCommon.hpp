#ifndef __COMMON_PARTICLECOMMON_HPP__
#define __COMMON_PARTICLECOMMON_HPP__

#include "DNACommon.hpp"

namespace Retro
{
namespace DNAParticle
{
extern LogVisor::LogModule LogModule;

struct IElement : BigYAML
{
    enum class EClassID : uint32_t
    {
        NONE = 'NONE',
        CNST = 'CNST'
    };
    Delete _d;
    virtual ~IElement() = default;
    virtual const char* ClassID() const=0;
};

struct IRealElement : IElement {Delete _d;};
struct RealElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IRealElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(Athena::io::YAMLDocReader& r);
    void write(Athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(Athena::io::IStreamReader& r);
    void write(Athena::io::IStreamWriter& w) const;
};

struct IIntElement : IElement {Delete _d;};
struct IntElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IIntElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(Athena::io::YAMLDocReader& r);
    void write(Athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(Athena::io::IStreamReader& r);
    void write(Athena::io::IStreamWriter& w) const;
};

struct IVectorElement : IElement {Delete _d;};
struct VectorElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IVectorElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(Athena::io::YAMLDocReader& r);
    void write(Athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(Athena::io::IStreamReader& r);
    void write(Athena::io::IStreamWriter& w) const;
};

struct IColorElement : IElement {Delete _d;};
struct ColorElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IColorElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(Athena::io::YAMLDocReader& r);
    void write(Athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(Athena::io::IStreamReader& r);
    void write(Athena::io::IStreamWriter& w) const;
};

struct IModVectorElement : IElement {Delete _d;};
struct ModVectorElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IModVectorElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(Athena::io::YAMLDocReader& r);
    void write(Athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(Athena::io::IStreamReader& r);
    void write(Athena::io::IStreamWriter& w) const;
};

struct IEmitterElement : IElement {Delete _d;};
struct EmitterElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IEmitterElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(Athena::io::YAMLDocReader& r);
    void write(Athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(Athena::io::IStreamReader& r);
    void write(Athena::io::IStreamWriter& w) const;
};

struct IUVElement : IElement {Delete _d;};

struct RELifetimeTween : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "LFTW";}
};

struct REConstant : IRealElement
{
    Delete _d;
    Value<float> val;

    void read(Athena::io::YAMLDocReader& r)
    {
        val = r.readFloat(nullptr);
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        w.writeFloat(nullptr, val);
    }
    size_t binarySize(size_t __isz) const
    {
        return __isz + 4;
    }
    void read(Athena::io::IStreamReader& r)
    {
        val = r.readFloatBig();
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        w.writeFloatBig(val);
    }

    const char* ClassID() const {return "CNST";}
};

struct RETimeChain : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    IntElementFactory c;
    const char* ClassID() const {return "CHAN";}
};

struct REAdd : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "ADD_";}
};

struct REClamp : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    const char* ClassID() const {return "CLMP";}
};

struct REKeyframeEmitter : IRealElement
{
    DECL_YAML
    Value<atUint32> percentage;
    Value<atUint32> b;
    Value<atUint8> c;
    Value<atUint8> d;
    Value<atUint32> e;
    Value<atUint32> f;
    Value<atUint32> count;
    Vector<float, DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentage ? "KEYP" : "KEYE";}
};

struct REInitialRandom : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "IRND";}
};

struct RERandom : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "RAND";}
};

struct REMultiply : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "MULT";}
};

struct REPulse : IRealElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    const char* ClassID() const {return "PULS";}
};

struct RETimeScale : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    const char* ClassID() const {return "SCAL";}
};

struct RELifetimePercent : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    const char* ClassID() const {return "RLPT";}
};

struct RESineWave : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    const char* ClassID() const {return "SINE";}
};

struct REISWT : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "ISWT";}
};

struct RECompareLessThan : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    const char* ClassID() const {return "CLTN";}
};

struct RECompareEquals : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    const char* ClassID() const {return "CEQL";}
};

struct REParticleAccessParam1 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP1";}
};

struct REParticleAccessParam2 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP2";}
};

struct REParticleAccessParam3 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP3";}
};

struct REParticleAccessParam4 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP4";}
};

struct REParticleAccessParam5 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP5";}
};

struct REParticleAccessParam6 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP6";}
};

struct REParticleAccessParam7 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP7";}
};

struct REParticleAccessParam8 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP8";}
};

struct REPSLL : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PSLL";}
};

struct REPRLW : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PRLW";}
};

struct RESubtract : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "SUB_";}
};

struct REVectorMagnitude : IRealElement
{
    DECL_YAML
    VectorElementFactory a;
    const char* ClassID() const {return "VMAG";}
};

struct REVectorXToReal : IRealElement
{
    DECL_YAML
    VectorElementFactory a;
    const char* ClassID() const {return "VXTR";}
};

struct REVectorYToReal : IRealElement
{
    DECL_YAML
    VectorElementFactory a;
    const char* ClassID() const {return "VYTR";}
};

struct REVectorZToReal : IRealElement
{
    DECL_YAML
    VectorElementFactory a;
    const char* ClassID() const {return "VZTR";}
};

struct RECEXT : IRealElement
{
    DECL_YAML
    IntElementFactory a;
    const char* ClassID() const {return "CEXT";}
};

struct REIntTimesReal : IRealElement
{
    DECL_YAML
    IntElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "ITRL";}
};

struct IEKeyframeEmitter : IIntElement
{
    DECL_YAML
    Value<atUint32> percentage;
    Value<atUint32> b;
    Value<atUint8> c;
    Value<atUint8> d;
    Value<atUint32> e;
    Value<atUint32> f;
    Value<atUint32> count;
    Vector<atUint32, DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentage ? "KEYP" : "KEYE";}
};

struct IEDeath : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "DETH";}
};

struct IEClamp : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory c;
    const char* ClassID() const {return "CLMP";}
};

struct IETimeChain : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory c;
    const char* ClassID() const {return "CHAN";}
};

struct IEAdd : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "ADD_";}
};

struct IEConstant : IIntElement
{
    Delete _d;
    Value<atUint32> val;

    void read(Athena::io::YAMLDocReader& r)
    {
        val = r.readUint32(nullptr);
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        w.writeUint32(nullptr, val);
    }
    size_t binarySize(size_t __isz) const
    {
        return __isz + 4;
    }
    void read(Athena::io::IStreamReader& r)
    {
        val = r.readUint32Big();
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        w.writeUint32Big(val);
    }

    const char* ClassID() const {return "CNST";}
};

struct IEImpulse : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    const char* ClassID() const {return "IMPL";}
};

struct IELifetimePercent : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    const char* ClassID() const {return "ILPT";}
};

struct IEInitialRandom : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "IRND";}
};

struct IEPulse : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory c;
    IntElementFactory d;
    const char* ClassID() const {return "PULS";}
};

struct IEMultiply : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "MULT";}
};

struct IESampleAndHold : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory c;
    const char* ClassID() const {return "SPAH";}
};

struct IERandom : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "RAND";}
};

struct IETimeScale : IIntElement
{
    DECL_YAML
    RealElementFactory a;
    const char* ClassID() const {return "TSCL";}
};

struct IEGTCP : IIntElement
{
    DECL_YAML
    const char* ClassID() const {return "GTCP";}
};

struct IEModulo : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "MODU";}
};

struct IESubtract : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "SUB_";}
};

struct VECone : IVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "CONE";}
};

struct VETimeChain : IVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    IntElementFactory c;
    const char* ClassID() const {return "CHAN";}
};

struct VEAngleCone : IVectorElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    RealElementFactory e;
    const char* ClassID() const {return "ANGC";}
};

struct VEAdd : IVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    const char* ClassID() const {return "ADD_";}
};

struct VECircleCluster : IVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    IntElementFactory c;
    RealElementFactory d;
    const char* ClassID() const {return "CCLU";}
};

struct VEConstant : IVectorElement
{
    Delete _d;
    RealElementFactory comps[3];

    void read(Athena::io::YAMLDocReader& r)
    {
        for (int i=0 ; i<3 ; ++i)
        {
            r.enterSubRecord(nullptr);
            comps[i].read(r);
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        w.enterSubVector(nullptr);
        for (int i=0 ; i<3 ; ++i)
        {
            w.enterSubRecord(nullptr);
            comps[i].write(w);
            w.leaveSubRecord();
        }
        w.leaveSubVector();
    }
    size_t binarySize(size_t __isz) const
    {
        __isz = comps[0].binarySize(__isz);
        __isz = comps[1].binarySize(__isz);
        return comps[2].binarySize(__isz);
    }
    void read(Athena::io::IStreamReader& r)
    {
        comps[0].read(r);
        comps[1].read(r);
        comps[2].read(r);
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        comps[0].write(w);
        comps[1].write(w);
        comps[2].write(w);
    }

    const char* ClassID() const {return "CNST";}
};

struct VECircle : IVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    RealElementFactory e;
    const char* ClassID() const {return "CIRC";}
};

struct VEKeyframeEmitter : IVectorElement
{
    DECL_YAML
    Value<atUint32> percentage;
    Value<atUint32> b;
    Value<atUint8> c;
    Value<atUint8> d;
    Value<atUint32> e;
    Value<atUint32> f;
    Value<atUint32> count;
    Vector<atVec3f, DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentage ? "KEYP" : "KEYE";}
};

struct VEMultiply : IVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    const char* ClassID() const {return "MULT";}
};

struct VERealToVector : IVectorElement
{
    DECL_YAML
    RealElementFactory a;
    const char* ClassID() const {return "RTOV";}
};

struct VEPulse : IVectorElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    VectorElementFactory c;
    VectorElementFactory d;
    const char* ClassID() const {return "PULS";}
};

struct VEParticleVelocity : IVectorElement
{
    DECL_YAML
    const char* ClassID() const {return "PVEL";}
};

struct VESPOS : IVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    const char* ClassID() const {return "SPOS";}
};

struct VEPLCO : IVectorElement
{
    DECL_YAML
    const char* ClassID() const {return "PLCO";}
};

struct VEPLOC : IVectorElement
{
    DECL_YAML
    const char* ClassID() const {return "PLOC";}
};

struct VEPSOR : IVectorElement
{
    DECL_YAML
    const char* ClassID() const {return "PSOR";}
};

struct VEPSOF : IVectorElement
{
    DECL_YAML
    const char* ClassID() const {return "PSOF";}
};

struct CEKeyframeEmitter : IColorElement
{
    DECL_YAML
    Value<atUint32> percentage;
    Value<atUint32> b;
    Value<atUint8> c;
    Value<atUint8> d;
    Value<atUint32> e;
    Value<atUint32> f;
    Value<atUint32> count;
    Vector<atVec4f, DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentage ? "KEYP" : "KEYE";}
};

struct CEConstant : IColorElement
{
    Delete _d;
    RealElementFactory comps[4];

    void read(Athena::io::YAMLDocReader& r)
    {
        for (int i=0 ; i<4 ; ++i)
        {
            r.enterSubRecord(nullptr);
            comps[i].read(r);
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        w.enterSubVector(nullptr);
        for (int i=0 ; i<4 ; ++i)
        {
            w.enterSubRecord(nullptr);
            comps[i].write(w);
            w.leaveSubRecord();
        }
        w.leaveSubVector();
    }
    size_t binarySize(size_t __isz) const
    {
        __isz = comps[0].binarySize(__isz);
        __isz = comps[1].binarySize(__isz);
        __isz = comps[2].binarySize(__isz);
        return comps[3].binarySize(__isz);
    }
    void read(Athena::io::IStreamReader& r)
    {
        comps[0].read(r);
        comps[1].read(r);
        comps[2].read(r);
        comps[3].read(r);
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        comps[0].write(w);
        comps[1].write(w);
        comps[2].write(w);
        comps[3].write(w);
    }

    const char* ClassID() const {return "CNST";}
};

struct CETimeChain : IColorElement
{
    DECL_YAML
    ColorElementFactory a;
    ColorElementFactory b;
    IntElementFactory c;
    const char* ClassID() const {return "CHAN";}
};

struct CEFadeEnd : IColorElement
{
    DECL_YAML
    ColorElementFactory a;
    ColorElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    const char* ClassID() const {return "CFDE";}
};

struct CEFade : IColorElement
{
    DECL_YAML
    ColorElementFactory a;
    ColorElementFactory b;
    RealElementFactory c;
    const char* ClassID() const {return "FADE";}
};

struct CEPulse : IColorElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    ColorElementFactory c;
    ColorElementFactory d;
    const char* ClassID() const {return "PULS";}
};

struct MVEImplosion : IModVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    Value<EClassID> boolCls = EClassID::CNST;
    Value<bool> boolVal;
    const char* ClassID() const {return "IMPL";}
};

struct MVEExponentialImplosion : IModVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    Value<EClassID> boolCls = EClassID::CNST;
    Value<bool> boolVal;
    const char* ClassID() const {return "EMPL";}
};

struct MVETimeChain : IModVectorElement
{
    DECL_YAML
    ModVectorElementFactory a;
    ModVectorElementFactory b;
    IntElementFactory c;
    const char* ClassID() const {return "CHAN";}
};

struct MVEBounce : IModVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    Value<EClassID> boolCls = EClassID::CNST;
    Value<bool> boolVal;
    const char* ClassID() const {return "BNCE";}
};

struct MVEConstant : IModVectorElement
{
    Delete _d;
    RealElementFactory comps[3];

    void read(Athena::io::YAMLDocReader& r)
    {
        for (int i=0 ; i<3 ; ++i)
        {
            r.enterSubRecord(nullptr);
            comps[i].read(r);
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        w.enterSubVector(nullptr);
        for (int i=0 ; i<3 ; ++i)
        {
            w.enterSubRecord(nullptr);
            comps[i].write(w);
            w.leaveSubRecord();
        }
        w.leaveSubVector();
    }
    size_t binarySize(size_t __isz) const
    {
        __isz = comps[0].binarySize(__isz);
        __isz = comps[1].binarySize(__isz);
        return comps[2].binarySize(__isz);
    }
    void read(Athena::io::IStreamReader& r)
    {
        comps[0].read(r);
        comps[1].read(r);
        comps[2].read(r);
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        comps[0].write(w);
        comps[1].write(w);
        comps[2].write(w);
    }

    const char* ClassID() const {return "CNST";}
};

struct MVEGravity : IModVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    const char* ClassID() const {return "GRAV";}
};

struct MVEExplode : IModVectorElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "EXPL";}
};

struct MVESetPosition : IModVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    const char* ClassID() const {return "SPOS";}
};

struct MVELinearImplosion : IModVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    Value<EClassID> boolCls = EClassID::CNST;
    Value<bool> boolVal;
    const char* ClassID() const {return "LMPL";}
};

struct MVEPulse : IModVectorElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    ModVectorElementFactory c;
    ModVectorElementFactory d;
    const char* ClassID() const {return "PULS";}
};

struct MVEWind : IModVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "WIND";}
};

struct MVESwirl : IModVectorElement
{
    DECL_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    const char* ClassID() const {return "SWRL";}
};

struct EESimpleEmitter : IEmitterElement
{
    DECL_YAML
    VectorElementFactory loc;
    VectorElementFactory vec;
    const char* ClassID() const {return "SEMR";}
};

struct VESphere : IEmitterElement
{
    DECL_YAML
    VectorElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    const char* ClassID() const {return "SPHE";}
};

struct VEAngleSphere : IEmitterElement
{
    DECL_YAML
    VectorElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    RealElementFactory e;
    RealElementFactory f;
    RealElementFactory g;
    const char* ClassID() const {return "ASPH";}
};

struct EESimpleEmitterTR : EESimpleEmitter
{
    Delete _d;

    void read(Athena::io::YAMLDocReader& r)
    {
        loc.m_elem.reset();
        vec.m_elem.reset();
        if (r.enterSubRecord("ILOC"))
        {
            loc.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("IVEC"))
        {
            vec.read(r);
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        w.enterSubRecord("ILOC");
        loc.write(w);
        w.leaveSubRecord();
        w.enterSubRecord("IVEC");
        vec.write(w);
        w.leaveSubRecord();
    }
    size_t binarySize(size_t __isz) const
    {
        __isz += 8;
        __isz = loc.binarySize(__isz);
        __isz = vec.binarySize(__isz);
        return __isz;
    }
    void read(Athena::io::IStreamReader& r)
    {
        loc.m_elem.reset();
        vec.m_elem.reset();
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('ILOC'))
        {
            loc.read(r);
            r.readBytesToBuf(&clsId, 4);
            if (clsId == SBIG('IVEC'))
                vec.read(r);
        }
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        w.writeBytes((atInt8*)"ILOC", 4);
        loc.write(w);
        w.writeBytes((atInt8*)"IVEC", 4);
        vec.write(w);
    }
    const char* ClassID() const {return "SETR";}
};

template <class IDType>
struct UVEConstant : IUVElement
{
    Delete _d;
    IDType tex;
    void read(Athena::io::YAMLDocReader& r)
    {
        tex.clear();
        if (r.enterSubRecord("tex"))
        {
            if (r.enterSubRecord("CNST"))
            {
                tex.read(r);
                r.leaveSubRecord();
            }
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        if (tex)
        {
            w.enterSubRecord("CNST");
            tex.write(w);
            w.leaveSubRecord();
        }
    }
    size_t binarySize(size_t __isz) const
    {
        if (tex)
            return tex.binarySize(__isz + 4);
        else
            return __isz + 4;
    }
    void read(Athena::io::IStreamReader& r)
    {
        tex.clear();
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('CNST'))
            tex.read(r);
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        if (tex)
        {
            w.writeBytes((atInt8*)"CNST", 4);
            tex.write(w);
        }
        else
            w.writeBytes((atInt8*)"NONE", 4);
    }
    const char* ClassID() const {return "CNST";}
};

template <class IDType>
struct UVEAnimTexture : IUVElement
{
    Delete _d;
    IDType tex;
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory c;
    IntElementFactory d;
    IntElementFactory e;
    Value<EClassID> boolCls = EClassID::CNST;
    Value<bool> boolVal;
    void read(Athena::io::YAMLDocReader& r)
    {
        tex.clear();
        if (r.enterSubRecord("tex"))
        {
            if (r.enterSubRecord("CNST"))
            {
                tex.read(r);
                r.leaveSubRecord();
            }
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("a"))
        {
            a.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("b"))
        {
            b.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("c"))
        {
            c.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("d"))
        {
            d.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("e"))
        {
            e.read(r);
            r.leaveSubRecord();
        }
        boolCls = EClassID::NONE;
        if (r.enterSubRecord("bool"))
        {
            if (r.enterSubRecord("CNST"))
            {
                boolCls = EClassID::CNST;
                boolVal = r.readBool(nullptr);
                r.leaveSubRecord();
            }
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        if (tex)
        {
            w.enterSubRecord("CNST");
            tex.write(w);
            w.leaveSubRecord();
        }
        a.write(w);
        b.write(w);
        c.write(w);
        d.write(w);
        e.write(w);
        w.enterSubRecord("bool");
        if (boolCls == EClassID::CNST)
        {
            w.enterSubRecord("CNST");
            w.writeBool(nullptr, boolVal);
            w.leaveSubRecord();
        }
        w.leaveSubRecord();
    }
    size_t binarySize(size_t __isz) const
    {
        __isz += 8;
        if (tex)
            __isz = tex.binarySize(__isz);
        __isz = a.binarySize(__isz);
        __isz = b.binarySize(__isz);
        __isz = c.binarySize(__isz);
        __isz = d.binarySize(__isz);
        __isz = e.binarySize(__isz);
        if (boolCls == EClassID::CNST)
            __isz += 1;
        return __isz;
    }
    void read(Athena::io::IStreamReader& r)
    {
        tex.clear();
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('CNST'))
            tex.read(r);
        a.read(r);
        b.read(r);
        c.read(r);
        d.read(r);
        e.read(r);
        boolCls = EClassID::NONE;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('CNST'))
        {
            boolCls = EClassID::CNST;
            boolVal = r.readBool();
        }
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        if (tex)
        {
            w.writeBytes((atInt8*)"CNST", 4);
            tex.write(w);
        }
        else
            w.writeBytes((atInt8*)"NONE", 4);
        a.write(w);
        b.write(w);
        c.write(w);
        d.write(w);
        e.write(w);
        if (boolCls == EClassID::CNST)
        {
            w.writeBytes((atInt8*)"CNST", 4);
            w.writeBool(boolVal);
        }
        else
            w.writeBytes((atInt8*)"NONE", 4);
    }
    const char* ClassID() const {return "ATEX";}
};

template <class IDType>
struct UVElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IUVElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(Athena::io::YAMLDocReader& r)
    {
        if (r.enterSubRecord("CNST"))
        {
            m_elem.reset(new struct UVEConstant<IDType>);
            m_elem->read(r);
            r.leaveSubRecord();
        }
        else if (r.enterSubRecord("ATEX"))
        {
            m_elem.reset(new struct UVEAnimTexture<IDType>);
            m_elem->read(r);
            r.leaveSubRecord();
        }
        else
            m_elem.reset();
    }

    void write(Athena::io::YAMLDocWriter& w) const
    {
        if (m_elem)
        {
            w.enterSubRecord(m_elem->ClassID());
            m_elem->write(w);
            w.leaveSubRecord();
        }
    }

    size_t binarySize(size_t __isz) const
    {
        if (m_elem)
            return m_elem->binarySize(__isz + 4);
        else
            return __isz + 4;
    }

    void read(Athena::io::IStreamReader& r)
    {
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        switch (clsId)
        {
        case SBIG('CNST'):
            m_elem.reset(new struct UVEConstant<IDType>);
            break;
        case SBIG('ATEX'):
            m_elem.reset(new struct UVEAnimTexture<IDType>);
            break;
        default:
            m_elem.reset();
            return;
        }
        m_elem->read(r);
    }

    void write(Athena::io::IStreamWriter& w) const
    {
        if (m_elem)
        {
            w.writeBytes((atInt8*)m_elem->ClassID(), 4);
            m_elem->write(w);
        }
        else
            w.writeBytes((atInt8*)"NONE", 4);
    }
};

template <class IDType>
struct SpawnSystemKeyframeData : BigYAML
{
    Delete _d;
    Value<atUint32> a;
    Value<atUint32> b;
    Value<atUint32> c;
    Value<atUint32> d;

    struct SpawnSystemKeyframeInfo : BigYAML
    {
        Delete _d;
        IDType id;
        Value<atUint32> a;
        Value<atUint32> b;
        Value<atUint32> c;

        void read(Athena::io::YAMLDocReader& r)
        {
            if (r.enterSubRecord("id"))
            {
                id.read(r);
                r.leaveSubRecord();
            }
            if (r.enterSubRecord("a"))
            {
                a = r.readUint32(nullptr);
                r.leaveSubRecord();
            }
            if (r.enterSubRecord("b"))
            {
                b = r.readUint32(nullptr);
                r.leaveSubRecord();
            }
            if (r.enterSubRecord("c"))
            {
                c = r.readUint32(nullptr);
                r.leaveSubRecord();
            }
        }
        void write(Athena::io::YAMLDocWriter& w) const
        {
            w.enterSubRecord("id");
            id.write(w);
            w.leaveSubRecord();
            w.writeUint32("a", a);
            w.writeUint32("b", b);
            w.writeUint32("c", c);
        }
        size_t binarySize(size_t __isz) const
        {
            return id.binarySize(__isz + 12);
        }
        void read(Athena::io::IStreamReader& r)
        {
            id.read(r);
            a = r.readUint32Big();
            b = r.readUint32Big();
            c = r.readUint32Big();
        }
        void write(Athena::io::IStreamWriter& w) const
        {
            id.write(w);
            w.writeUint32Big(a);
            w.writeUint32Big(b);
            w.writeUint32Big(c);
        }
    };

    std::vector<std::pair<atUint32, std::vector<SpawnSystemKeyframeInfo>>> spawns;

    void read(Athena::io::YAMLDocReader& r)
    {
        if (r.enterSubRecord("a"))
        {
            a = r.readUint32(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("b"))
        {
            b = r.readUint32(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("c"))
        {
            c = r.readUint32(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("d"))
        {
            d = r.readUint32(nullptr);
            r.leaveSubRecord();
        }
        spawns.clear();
        if (r.enterSubVector("spawns"))
        {
            spawns.reserve(r.getCurNode()->m_seqChildren.size());
            for (const auto& child : r.getCurNode()->m_seqChildren)
            {
                if (r.enterSubRecord(nullptr))
                {
                    spawns.emplace_back();
                    spawns.back().first = r.readUint32("first");
                    if (r.enterSubVector("second"))
                    {
                        spawns.back().second.reserve(r.getCurNode()->m_seqChildren.size());
                        for (const auto& in : r.getCurNode()->m_seqChildren)
                        {
                            spawns.back().second.emplace_back();
                            SpawnSystemKeyframeInfo& info = spawns.back().second.back();
                            r.enterSubRecord(nullptr);
                            info.read(r);
                            r.leaveSubRecord();
                        }
                        r.leaveSubVector();
                    }
                    r.leaveSubRecord();
                }
            }
            r.leaveSubVector();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        if (spawns.empty())
            return;
        w.writeUint32("a", a);
        w.writeUint32("b", b);
        w.writeUint32("c", c);
        w.writeUint32("d", d);
        w.enterSubVector("spawns");
        for (const auto& spawn : spawns)
        {
            w.enterSubRecord(nullptr);
            w.writeUint32("first", spawn.first);
            w.enterSubVector("second");
            for (const auto& info : spawn.second)
            {
                w.enterSubRecord(nullptr);
                info.write(w);
                w.leaveSubRecord();
            }
            w.leaveSubVector();
            w.leaveSubRecord();
        }
        w.leaveSubVector();
    }
    size_t binarySize(size_t __isz) const
    {
        __isz += 20;
        for (const auto& spawn : spawns)
        {
            __isz += 8;
            for (const auto& info : spawn.second)
                __isz = info.binarySize(__isz);
        }
        return __isz;
    }
    void read(Athena::io::IStreamReader& r)
    {
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId != SBIG('CNST'))
            return;

        a = r.readUint32Big();
        b = r.readUint32Big();
        c = r.readUint32Big();
        d = r.readUint32Big();
        uint32_t count = r.readUint32Big();
        spawns.clear();
        spawns.reserve(count);
        for (size_t i=0 ; i<count ; ++i)
        {
            spawns.emplace_back();
            spawns.back().first = r.readUint32Big();
            uint32_t infoCount = r.readUint32Big();
            spawns.back().second.reserve(infoCount);
            for (size_t j=0 ; j<infoCount ; ++j)
            {
                spawns.back().second.emplace_back();
                spawns.back().second.back().read(r);
            }
        }
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        if (spawns.empty())
        {
            w.writeBytes((atInt8*)"NONE", 4);
            return;
        }
        w.writeBytes((atInt8*)"CNST", 4);
        w.writeUint32Big(a);
        w.writeUint32Big(b);
        w.writeUint32Big(c);
        w.writeUint32Big(d);
        w.writeUint32Big(spawns.size());
        for (const auto& spawn : spawns)
        {
            w.writeUint32Big(spawn.first);
            w.writeUint32Big(spawn.second.size());
            for (const auto& info : spawn.second)
                info.write(w);
        }
    }

    operator bool() const {return spawns.size() != 0;}
};

template <class IDType>
struct ChildResourceFactory : BigYAML
{
    Delete _d;
    IDType id;
    void read(Athena::io::YAMLDocReader& r)
    {
        id.clear();
        if (r.enterSubRecord("CNST"))
        {
            id.read(r);
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {
        if (id)
        {
            w.enterSubRecord("CNST");
            id.write(w);
            w.leaveSubRecord();
        }
    }
    size_t binarySize(size_t __isz) const
    {
        if (id)
            return id.binarySize(__isz + 4);
        else
            return __isz + 4;
    }
    void read(Athena::io::IStreamReader& r)
    {
        id.clear();
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('CNST'))
            id.read(r);
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        if (id)
        {
            w.writeBytes((atInt8*)"CNST", 4);
            id.write(w);
        }
        else
            w.writeBytes((atInt8*)"NONE", 4);
    }

    operator bool() const {return id.operator bool();}
};

}
}

#endif // __COMMON_PARTICLECOMMON_HPP__
