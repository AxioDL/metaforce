#ifndef __COMMON_PARTICLECOMMON_HPP__
#define __COMMON_PARTICLECOMMON_HPP__

#include "DNACommon.hpp"

namespace DataSpec
{
namespace DNAParticle
{
extern logvisor::Module LogModule;

struct IElement : BigYAML
{
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

    void read(athena::io::YAMLDocReader& r);
    void write(athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(athena::io::IStreamReader& r);
    void write(athena::io::IStreamWriter& w) const;
};

struct IIntElement : IElement {Delete _d;};
struct IntElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IIntElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(athena::io::YAMLDocReader& r);
    void write(athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(athena::io::IStreamReader& r);
    void write(athena::io::IStreamWriter& w) const;
};

struct IVectorElement : IElement {Delete _d;};
struct VectorElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IVectorElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(athena::io::YAMLDocReader& r);
    void write(athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(athena::io::IStreamReader& r);
    void write(athena::io::IStreamWriter& w) const;
};

struct IColorElement : IElement {Delete _d;};
struct ColorElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IColorElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(athena::io::YAMLDocReader& r);
    void write(athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(athena::io::IStreamReader& r);
    void write(athena::io::IStreamWriter& w) const;
};

struct IModVectorElement : IElement {Delete _d;};
struct ModVectorElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IModVectorElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(athena::io::YAMLDocReader& r);
    void write(athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(athena::io::IStreamReader& r);
    void write(athena::io::IStreamWriter& w) const;
};

struct IEmitterElement : IElement {Delete _d;};
struct EmitterElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IEmitterElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(athena::io::YAMLDocReader& r);
    void write(athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(athena::io::IStreamReader& r);
    void write(athena::io::IStreamWriter& w) const;
};

struct IUVElement : IElement
{
    Delete _d;
    virtual void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const=0;
};

struct BoolHelper : IElement
{
    Delete _d;
    bool value = false;
    operator bool() const {return value;}
    BoolHelper& operator=(bool val) {value = val; return *this;}
    void read(athena::io::YAMLDocReader& r)
    {
        value = r.readBool(nullptr);
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        w.writeBool(nullptr, value);
    }
    size_t binarySize(size_t __isz) const
    {
        return __isz + 5;
    }
    void read(athena::io::IStreamReader& r)
    {
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('CNST'))
            value = r.readBool();
        else
            value = false;
    }
    void write(athena::io::IStreamWriter& w) const
    {
        w.writeBytes((atInt8*)"CNST", 4);
        w.writeBool(value);
    }
    const char* ClassID() const {return "BoolHelper";}
};

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

    void read(athena::io::YAMLDocReader& r)
    {
        val = r.readFloat(nullptr);
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        w.writeFloat(nullptr, val);
    }
    size_t binarySize(size_t __isz) const
    {
        return __isz + 4;
    }
    void read(athena::io::IStreamReader& r)
    {
        val = r.readFloatBig();
    }
    void write(athena::io::IStreamWriter& w) const
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
    IntElementFactory thresholdFrame;
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
    RealElementFactory min;
    RealElementFactory max;
    RealElementFactory val;
    const char* ClassID() const {return "CLMP";}
};

struct REKeyframeEmitter : IRealElement
{
    DECL_YAML
    Value<atUint32> percentageTween;
    Value<atUint32> unk1;
    Value<bool> loop;
    Value<atUint8> unk2;
    Value<atUint32> loopEnd;
    Value<atUint32> loopStart;
    Value<atUint32> count;
    Vector<float, DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentageTween ? "KEYP" : "KEYE";}
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
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "PULS";}
};

struct RETimeScale : IRealElement
{
    DECL_YAML
    RealElementFactory dv;
    const char* ClassID() const {return "SCAL";}
};

struct RELifetimePercent : IRealElement
{
    DECL_YAML
    RealElementFactory percent;
    const char* ClassID() const {return "RLPT";}
};

struct RESineWave : IRealElement
{
    DECL_YAML
    RealElementFactory magnitude;
    RealElementFactory linearAngle;
    RealElementFactory constantAngle;
    const char* ClassID() const {return "SINE";}
};

struct REInitialSwitch : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "ISWT";}
};

struct RECompareLessThan : IRealElement
{
    DECL_YAML
    RealElementFactory ca;
    RealElementFactory cb;
    RealElementFactory pass;
    RealElementFactory fail;
    const char* ClassID() const {return "CLTN";}
};

struct RECompareEquals : IRealElement
{
    DECL_YAML
    RealElementFactory ca;
    RealElementFactory cb;
    RealElementFactory pass;
    RealElementFactory fail;
    const char* ClassID() const {return "CEQL";}
};

struct REParticleAdvanceParam1 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP1";}
};

struct REParticleAdvanceParam2 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP2";}
};

struct REParticleAdvanceParam3 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP3";}
};

struct REParticleAdvanceParam4 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP4";}
};

struct REParticleAdvanceParam5 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP5";}
};

struct REParticleAdvanceParam6 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP6";}
};

struct REParticleAdvanceParam7 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP7";}
};

struct REParticleAdvanceParam8 : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PAP8";}
};

struct REParticleSizeOrLineLength : IRealElement
{
    DECL_YAML
    const char* ClassID() const {return "PSLL";}
};

struct REParticleRotationOrLineWidth : IRealElement
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
    VectorElementFactory vec;
    const char* ClassID() const {return "VMAG";}
};

struct REVectorXToReal : IRealElement
{
    DECL_YAML
    VectorElementFactory vec;
    const char* ClassID() const {return "VXTR";}
};

struct REVectorYToReal : IRealElement
{
    DECL_YAML
    VectorElementFactory vec;
    const char* ClassID() const {return "VYTR";}
};

struct REVectorZToReal : IRealElement
{
    DECL_YAML
    VectorElementFactory vec;
    const char* ClassID() const {return "VZTR";}
};

struct RECEXT : IRealElement
{
    DECL_YAML
    IntElementFactory index;
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
    Value<atUint32> percentageTween;
    Value<atUint32> unk1;
    Value<bool> loop;
    Value<atUint8> unk2;
    Value<atUint32> loopEnd;
    Value<atUint32> loopStart;
    Value<atUint32> count;
    Vector<atUint32, DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentageTween ? "KEYP" : "KEYE";}
};

struct IEDeath : IIntElement
{
    DECL_YAML
    IntElementFactory passthrough;
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "DETH";}
};

struct IEClamp : IIntElement
{
    DECL_YAML
    IntElementFactory min;
    IntElementFactory max;
    IntElementFactory val;
    const char* ClassID() const {return "CLMP";}
};

struct IETimeChain : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory thresholdFrame;
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

    void read(athena::io::YAMLDocReader& r)
    {
        val = r.readUint32(nullptr);
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        w.writeUint32(nullptr, val);
    }
    size_t binarySize(size_t __isz) const
    {
        return __isz + 4;
    }
    void read(athena::io::IStreamReader& r)
    {
        val = r.readUint32Big();
    }
    void write(athena::io::IStreamWriter& w) const
    {
        w.writeUint32Big(val);
    }

    const char* ClassID() const {return "CNST";}
};

struct IEImpulse : IIntElement
{
    DECL_YAML
    IntElementFactory val;
    const char* ClassID() const {return "IMPL";}
};

struct IELifetimePercent : IIntElement
{
    DECL_YAML
    IntElementFactory percent;
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
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    IntElementFactory a;
    IntElementFactory b;
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
    IntElementFactory val;
    IntElementFactory waitMin;
    IntElementFactory waitMax;
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
    RealElementFactory dv;
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
    IntElementFactory direction;
    IntElementFactory baseRadius;
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
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "CHAN";}
};

struct VEAngleCone : IVectorElement
{
    DECL_YAML
    RealElementFactory angleXBias;
    RealElementFactory angleYBias;
    RealElementFactory angleXRange;
    RealElementFactory angleYRange;
    RealElementFactory magnitude;
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
    VectorElementFactory circleOffset;
    VectorElementFactory circleNormal;
    IntElementFactory cycleFrames;
    RealElementFactory randomFactor;
    const char* ClassID() const {return "CCLU";}
};

struct VEConstant : IVectorElement
{
    Delete _d;
    RealElementFactory comps[3];

    void read(athena::io::YAMLDocReader& r)
    {
        size_t elemCount;
        if (auto v = r.enterSubVector(nullptr, elemCount))
        {
            for (int i=0 ; i<3 && i<elemCount ; ++i)
            {
                if (auto rec = r.enterSubRecord(nullptr))
                    comps[i].read(r);
            }
        }
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        if (auto v = w.enterSubVector(nullptr))
            for (int i=0 ; i<3 ; ++i)
                if (auto rec = w.enterSubRecord(nullptr))
                    comps[i].write(w);
    }
    size_t binarySize(size_t __isz) const
    {
        __isz = comps[0].binarySize(__isz);
        __isz = comps[1].binarySize(__isz);
        return comps[2].binarySize(__isz);
    }
    void read(athena::io::IStreamReader& r)
    {
        comps[0].read(r);
        comps[1].read(r);
        comps[2].read(r);
    }
    void write(athena::io::IStreamWriter& w) const
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
    VectorElementFactory circleOffset;
    VectorElementFactory circleNormal;
    RealElementFactory angleConstant;
    RealElementFactory angleLinear;
    RealElementFactory circleRadius;
    const char* ClassID() const {return "CIRC";}
};

struct VEKeyframeEmitter : IVectorElement
{
    DECL_YAML
    Value<atUint32> percentageTween;
    Value<atUint32> unk1;
    Value<bool> loop;
    Value<atUint8> unk2;
    Value<atUint32> loopEnd;
    Value<atUint32> loopStart;
    Value<atUint32> count;
    Vector<atVec3f, DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentageTween ? "KEYP" : "KEYE";}
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
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    VectorElementFactory a;
    VectorElementFactory b;
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
    Value<atUint32> percentageTween;
    Value<atUint32> unk1;
    Value<bool> loop;
    Value<atUint8> unk2;
    Value<atUint32> loopEnd;
    Value<atUint32> loopStart;
    Value<atUint32> count;
    Vector<atVec4f, DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentageTween ? "KEYP" : "KEYE";}
};

struct CEConstant : IColorElement
{
    Delete _d;
    RealElementFactory comps[4];

    void read(athena::io::YAMLDocReader& r)
    {
        for (int i=0 ; i<4 ; ++i)
            if (auto rec = r.enterSubRecord(nullptr))
                comps[i].read(r);
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        if (auto v = w.enterSubVector(nullptr))
            for (int i=0 ; i<4 ; ++i)
                if (auto rec = w.enterSubRecord(nullptr))
                    comps[i].write(w);
    }
    size_t binarySize(size_t __isz) const
    {
        __isz = comps[0].binarySize(__isz);
        __isz = comps[1].binarySize(__isz);
        __isz = comps[2].binarySize(__isz);
        return comps[3].binarySize(__isz);
    }
    void read(athena::io::IStreamReader& r)
    {
        comps[0].read(r);
        comps[1].read(r);
        comps[2].read(r);
        comps[3].read(r);
    }
    void write(athena::io::IStreamWriter& w) const
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
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "CHAN";}
};

struct CEFadeEnd : IColorElement
{
    DECL_YAML
    ColorElementFactory a;
    ColorElementFactory b;
    RealElementFactory startFrame;
    RealElementFactory endFrame;
    const char* ClassID() const {return "CFDE";}
};

struct CEFade : IColorElement
{
    DECL_YAML
    ColorElementFactory a;
    ColorElementFactory b;
    RealElementFactory endFrame;
    const char* ClassID() const {return "FADE";}
};

struct CEPulse : IColorElement
{
    DECL_YAML
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    ColorElementFactory a;
    ColorElementFactory b;
    const char* ClassID() const {return "PULS";}
};

struct MVEImplosion : IModVectorElement
{
    DECL_YAML
    VectorElementFactory implodePoint;
    RealElementFactory velocityScale;
    RealElementFactory maxRadius;
    RealElementFactory minRadius;
    BoolHelper enableMinRadius;
    const char* ClassID() const {return "IMPL";}
};

struct MVEExponentialImplosion : IModVectorElement
{
    DECL_YAML
    VectorElementFactory implodePoint;
    RealElementFactory velocityScale;
    RealElementFactory maxRadius;
    RealElementFactory minRadius;
    BoolHelper enableMinRadius;
    const char* ClassID() const {return "EMPL";}
};

struct MVETimeChain : IModVectorElement
{
    DECL_YAML
    ModVectorElementFactory a;
    ModVectorElementFactory b;
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "CHAN";}
};

struct MVEBounce : IModVectorElement
{
    DECL_YAML
    VectorElementFactory planePoint;
    VectorElementFactory planeNormal;
    RealElementFactory friction;
    RealElementFactory restitution;
    BoolHelper dieOnPenetrate;
    const char* ClassID() const {return "BNCE";}
};

struct MVEConstant : IModVectorElement
{
    Delete _d;
    RealElementFactory comps[3];

    void read(athena::io::YAMLDocReader& r)
    {
        for (int i=0 ; i<3 ; ++i)
            if (auto rec = r.enterSubRecord(nullptr))
                comps[i].read(r);
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        if (auto v = w.enterSubVector(nullptr))
            for (int i=0 ; i<3 ; ++i)
                if (auto rec = w.enterSubRecord(nullptr))
                    comps[i].write(w);
    }
    size_t binarySize(size_t __isz) const
    {
        __isz = comps[0].binarySize(__isz);
        __isz = comps[1].binarySize(__isz);
        return comps[2].binarySize(__isz);
    }
    void read(athena::io::IStreamReader& r)
    {
        comps[0].read(r);
        comps[1].read(r);
        comps[2].read(r);
    }
    void write(athena::io::IStreamWriter& w) const
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
    VectorElementFactory acceleration;
    const char* ClassID() const {return "GRAV";}
};

struct MVEExplode : IModVectorElement
{
    DECL_YAML
    RealElementFactory impulseMagnitude;
    RealElementFactory falloffFactor;
    const char* ClassID() const {return "EXPL";}
};

struct MVESetPosition : IModVectorElement
{
    DECL_YAML
    VectorElementFactory position;
    const char* ClassID() const {return "SPOS";}
};

struct MVELinearImplosion : IModVectorElement
{
    DECL_YAML
    VectorElementFactory implodePoint;
    RealElementFactory velocityScale;
    RealElementFactory maxRadius;
    RealElementFactory minRadius;
    BoolHelper enableMinRadius;
    const char* ClassID() const {return "LMPL";}
};

struct MVEPulse : IModVectorElement
{
    DECL_YAML
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    ModVectorElementFactory a;
    ModVectorElementFactory b;
    const char* ClassID() const {return "PULS";}
};

struct MVEWind : IModVectorElement
{
    DECL_YAML
    VectorElementFactory windVelocity;
    RealElementFactory factor;
    const char* ClassID() const {return "WIND";}
};

struct MVESwirl : IModVectorElement
{
    DECL_YAML
    VectorElementFactory helixPoint;
    VectorElementFactory curveBinormal;
    RealElementFactory targetRadius;
    RealElementFactory tangentialVelocity;
    const char* ClassID() const {return "SWRL";}
};

struct EESimpleEmitter : IEmitterElement
{
    DECL_YAML
    VectorElementFactory position;
    VectorElementFactory velocity;
    const char* ClassID() const {return "SEMR";}
};

struct VESphere : IEmitterElement
{
    DECL_YAML
    VectorElementFactory sphereOrigin;
    RealElementFactory sphereRadius;
    RealElementFactory magnitude;
    const char* ClassID() const {return "SPHE";}
};

struct VEAngleSphere : IEmitterElement
{
    DECL_YAML
    VectorElementFactory sphereOrigin;
    RealElementFactory sphereRadius;
    RealElementFactory magnitude;
    RealElementFactory angleXBias;
    RealElementFactory angleYBias;
    RealElementFactory angleXRange;
    RealElementFactory angleYRange;
    const char* ClassID() const {return "ASPH";}
};

struct EESimpleEmitterTR : EESimpleEmitter
{
    Delete _d;

    void read(athena::io::YAMLDocReader& r)
    {
        position.m_elem.reset();
        velocity.m_elem.reset();
        if (auto rec = r.enterSubRecord("ILOC"))
            position.read(r);
        if (auto rec = r.enterSubRecord("IVEC"))
            velocity.read(r);
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        if (auto rec = w.enterSubRecord("ILOC"))
            position.write(w);
        if (auto rec = w.enterSubRecord("IVEC"))
            velocity.write(w);
    }
    size_t binarySize(size_t __isz) const
    {
        __isz += 8;
        __isz = position.binarySize(__isz);
        __isz = velocity.binarySize(__isz);
        return __isz;
    }
    void read(athena::io::IStreamReader& r)
    {
        position.m_elem.reset();
        velocity.m_elem.reset();
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('ILOC'))
        {
            position.read(r);
            r.readBytesToBuf(&clsId, 4);
            if (clsId == SBIG('IVEC'))
                velocity.read(r);
        }
    }
    void write(athena::io::IStreamWriter& w) const
    {
        w.writeBytes((atInt8*)"ILOC", 4);
        position.write(w);
        w.writeBytes((atInt8*)"IVEC", 4);
        velocity.write(w);
    }
    const char* ClassID() const {return "SETR";}
};

template <class IDType>
struct UVEConstant : IUVElement
{
    Delete _d;
    IDType tex;
    void read(athena::io::YAMLDocReader& r)
    {
        tex.clear();
        if (auto rec = r.enterSubRecord("tex"))
            tex.read(r);
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        if (auto rec = w.enterSubRecord("tex"))
            tex.write(w);
    }
    size_t binarySize(size_t __isz) const
    {
        return tex.binarySize(__isz + 4);
    }
    void read(athena::io::IStreamReader& r)
    {
        tex.clear();
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('CNST'))
            tex.read(r);
    }
    void write(athena::io::IStreamWriter& w) const
    {
        w.writeBytes((atInt8*)"CNST", 4);
        tex.write(w);
    }
    const char* ClassID() const {return "CNST";}

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(tex, pathsOut);
    }
};

template <class IDType>
struct UVEAnimTexture : IUVElement
{
    Delete _d;
    IDType tex;
    IntElementFactory tileW;
    IntElementFactory tileH;
    IntElementFactory strideW;
    IntElementFactory strideH;
    IntElementFactory cycleFrames;
    Value<bool> loop = false;
    void read(athena::io::YAMLDocReader& r)
    {
        tex.clear();
        if (auto rec = r.enterSubRecord("tex"))
            tex.read(r);
        if (auto rec = r.enterSubRecord("tileW"))
            tileW.read(r);
        if (auto rec = r.enterSubRecord("tileH"))
            tileH.read(r);
        if (auto rec = r.enterSubRecord("strideW"))
            strideW.read(r);
        if (auto rec = r.enterSubRecord("strideH"))
            strideH.read(r);
        if (auto rec = r.enterSubRecord("cycleFrames"))
            cycleFrames.read(r);
        if (auto rec = r.enterSubRecord("loop"))
            loop = r.readBool(nullptr);
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        if (auto rec = w.enterSubRecord("tex"))
            tex.write(w);
        if (auto rec = w.enterSubRecord("tileW"))
            tileW.write(w);
        if (auto rec = w.enterSubRecord("tileH"))
            tileH.write(w);
        if (auto rec = w.enterSubRecord("strideW"))
            strideW.write(w);
        if (auto rec = w.enterSubRecord("strideH"))
            strideH.write(w);
        if (auto rec = w.enterSubRecord("cycleFrames"))
            cycleFrames.write(w);
        w.writeBool("loop", loop);
    }
    size_t binarySize(size_t __isz) const
    {
        __isz += 9;
        __isz = tex.binarySize(__isz);
        __isz = tileW.binarySize(__isz);
        __isz = tileH.binarySize(__isz);
        __isz = strideW.binarySize(__isz);
        __isz = strideH.binarySize(__isz);
        __isz = cycleFrames.binarySize(__isz);
        return __isz;
    }
    void read(athena::io::IStreamReader& r)
    {
        tex.clear();
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('CNST'))
            tex.read(r);
        tileW.read(r);
        tileH.read(r);
        strideW.read(r);
        strideH.read(r);
        cycleFrames.read(r);
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('CNST'))
            loop = r.readBool();
    }
    void write(athena::io::IStreamWriter& w) const
    {
        w.writeBytes((atInt8*)"CNST", 4);
        tex.write(w);
        tileW.write(w);
        tileH.write(w);
        strideW.write(w);
        strideH.write(w);
        cycleFrames.write(w);
        w.writeBytes((atInt8*)"CNST", 4);
        w.writeBool(loop);
    }
    const char* ClassID() const {return "ATEX";}

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(tex, pathsOut);
    }
};

template <class IDType>
struct UVElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IUVElement> m_elem;
    operator bool() const {return m_elem.operator bool();}

    void read(athena::io::YAMLDocReader& r)
    {
        if (auto rec = r.enterSubRecord("CNST"))
        {
            m_elem.reset(new struct UVEConstant<IDType>);
            m_elem->read(r);
        }
        else if (auto rec = r.enterSubRecord("ATEX"))
        {
            m_elem.reset(new struct UVEAnimTexture<IDType>);
            m_elem->read(r);
        }
        else
            m_elem.reset();
    }

    void write(athena::io::YAMLDocWriter& w) const
    {
        if (m_elem)
            if (auto rec = w.enterSubRecord(m_elem->ClassID()))
                m_elem->write(w);
    }

    size_t binarySize(size_t __isz) const
    {
        if (m_elem)
            return m_elem->binarySize(__isz + 4);
        else
            return __isz + 4;
    }

    void read(athena::io::IStreamReader& r)
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

    void write(athena::io::IStreamWriter& w) const
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
    Value<atUint32> endFrame;
    Value<atUint32> d;

    struct SpawnSystemKeyframeInfo : BigYAML
    {
        Delete _d;
        IDType id;
        Value<atUint32> a;
        Value<atUint32> b;
        Value<atUint32> c;

        void read(athena::io::YAMLDocReader& r)
        {
            if (auto rec = r.enterSubRecord("id"))
                id.read(r);
            if (auto rec = r.enterSubRecord("a"))
                a = r.readUint32(nullptr);
            if (auto rec = r.enterSubRecord("b"))
                b = r.readUint32(nullptr);
            if (auto rec = r.enterSubRecord("c"))
                c = r.readUint32(nullptr);
        }
        void write(athena::io::YAMLDocWriter& w) const
        {
            if (auto rec = w.enterSubRecord("id"))
                id.write(w);
            w.writeUint32("a", a);
            w.writeUint32("b", b);
            w.writeUint32("c", c);
        }
        size_t binarySize(size_t __isz) const
        {
            return id.binarySize(__isz + 12);
        }
        void read(athena::io::IStreamReader& r)
        {
            id.read(r);
            a = r.readUint32Big();
            b = r.readUint32Big();
            c = r.readUint32Big();
        }
        void write(athena::io::IStreamWriter& w) const
        {
            id.write(w);
            w.writeUint32Big(a);
            w.writeUint32Big(b);
            w.writeUint32Big(c);
        }
    };

    std::vector<std::pair<atUint32, std::vector<SpawnSystemKeyframeInfo>>> spawns;

    void read(athena::io::YAMLDocReader& r)
    {
        if (auto rec = r.enterSubRecord("a"))
            a = r.readUint32(nullptr);
        if (auto rec = r.enterSubRecord("b"))
            b = r.readUint32(nullptr);
        if (auto rec = r.enterSubRecord("endFrame"))
            endFrame = r.readUint32(nullptr);
        if (auto rec = r.enterSubRecord("d"))
            d = r.readUint32(nullptr);
        spawns.clear();
        size_t spawnCount;
        if (auto v = r.enterSubVector("spawns", spawnCount))
        {
            spawns.reserve(spawnCount);
            for (const auto& child : r.getCurNode()->m_seqChildren)
            {
                if (auto rec = r.enterSubRecord(nullptr))
                {
                    spawns.emplace_back();
                    spawns.back().first = r.readUint32("startFrame");
                    size_t systemCount;
                    if (auto v = r.enterSubVector("systems", systemCount))
                    {
                        spawns.back().second.reserve(systemCount);
                        for (const auto& in : r.getCurNode()->m_seqChildren)
                        {
                            spawns.back().second.emplace_back();
                            SpawnSystemKeyframeInfo& info = spawns.back().second.back();
                            if (auto rec = r.enterSubRecord(nullptr))
                                info.read(r);
                        }
                    }
                }
            }
        }
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        if (spawns.empty())
            return;
        w.writeUint32("a", a);
        w.writeUint32("b", b);
        w.writeUint32("endFrame", endFrame);
        w.writeUint32("d", d);
        if (auto v = w.enterSubVector("spawns"))
        {
            for (const auto& spawn : spawns)
            {
                if (auto rec = w.enterSubRecord(nullptr))
                {
                    w.writeUint32("startFrame", spawn.first);
                    if (auto v = w.enterSubVector("systems"))
                        for (const auto& info : spawn.second)
                            if (auto rec = w.enterSubRecord(nullptr))
                                info.write(w);
                }
            }
        }
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
    void read(athena::io::IStreamReader& r)
    {
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId != SBIG('CNST'))
            return;

        a = r.readUint32Big();
        b = r.readUint32Big();
        endFrame = r.readUint32Big();
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
    void write(athena::io::IStreamWriter& w) const
    {
        if (spawns.empty())
        {
            w.writeBytes((atInt8*)"NONE", 4);
            return;
        }
        w.writeBytes((atInt8*)"CNST", 4);
        w.writeUint32Big(a);
        w.writeUint32Big(b);
        w.writeUint32Big(endFrame);
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

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        for (const auto& p : spawns)
            for (const SpawnSystemKeyframeInfo& info : p.second)
                g_curSpec->flattenDependencies(info.id, pathsOut);
    }
};

template <class IDType>
struct ChildResourceFactory : BigYAML
{
    Delete _d;
    IDType id;
    void read(athena::io::YAMLDocReader& r)
    {
        id.clear();
        if (auto rec = r.enterSubRecord("CNST"))
            id.read(r);
    }
    void write(athena::io::YAMLDocWriter& w) const
    {
        if (id)
            if (auto rec = w.enterSubRecord("CNST"))
                id.write(w);
    }
    size_t binarySize(size_t __isz) const
    {
        if (id)
            return id.binarySize(__isz + 4);
        else
            return __isz + 4;
    }
    void read(athena::io::IStreamReader& r)
    {
        id.clear();
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId == SBIG('CNST'))
            id.read(r);
    }
    void write(athena::io::IStreamWriter& w) const
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
