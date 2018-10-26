#pragma once

#include "DNACommon.hpp"

namespace DataSpec::DNAParticle
{
extern logvisor::Module LogModule;

struct IElement : BigDNAVYaml
{
    Delete _d;
    virtual ~IElement() = default;
    virtual const char* ClassID() const=0;
    const char* DNATypeV() const {return ClassID();}
};

struct IRealElement : IElement {Delete _d2;};
struct RealElementFactory : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    std::unique_ptr<IRealElement> m_elem;
    operator bool() const {return m_elem.operator bool();}
};

struct IIntElement : IElement {Delete _d2;};
struct IntElementFactory : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    std::unique_ptr<IIntElement> m_elem;
    operator bool() const {return m_elem.operator bool();}
};

struct IVectorElement : IElement {Delete _d2;};
struct VectorElementFactory : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    std::unique_ptr<IVectorElement> m_elem;
    operator bool() const {return m_elem.operator bool();}
};

struct IColorElement : IElement {Delete _d2;};
struct ColorElementFactory : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    std::unique_ptr<IColorElement> m_elem;
    operator bool() const {return m_elem.operator bool();}
};

struct IModVectorElement : IElement {Delete _d2;};
struct ModVectorElementFactory : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    std::unique_ptr<IModVectorElement> m_elem;
    operator bool() const {return m_elem.operator bool();}
};

struct IEmitterElement : IElement {Delete _d2;};
struct EmitterElementFactory : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    std::unique_ptr<IEmitterElement> m_elem;
    operator bool() const {return m_elem.operator bool();}
};

struct IUVElement : IElement
{
    Delete _d2;
    virtual void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const=0;
};

struct BoolHelper : IElement
{
    AT_DECL_EXPLICIT_DNA_YAML
    bool value = false;
    operator bool() const {return value;}
    BoolHelper& operator=(bool val) {value = val; return *this;}
    const char* ClassID() const {return "BoolHelper";}
};

struct RELifetimeTween : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "LFTW";}
};

struct REConstant : IRealElement
{
    AT_DECL_EXPLICIT_DNA_YAML
    Value<float> val;
    const char* ClassID() const {return "CNST";}
};

struct RETimeChain : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory a;
    RealElementFactory b;
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "CHAN";}
};

struct REAdd : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "ADD_";}
};

struct REClamp : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory min;
    RealElementFactory max;
    RealElementFactory val;
    const char* ClassID() const {return "CLMP";}
};

struct REKeyframeEmitter : IRealElement
{
    AT_DECL_DNA_YAML
    Value<atUint32> percentageTween;
    Value<atUint32> unk1;
    Value<bool> loop;
    Value<atUint8> unk2;
    Value<atUint32> loopEnd;
    Value<atUint32> loopStart;
    Value<atUint32> count;
    Vector<float, AT_DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentageTween ? "KEYP" : "KEYE";}
};

struct REInitialRandom : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "IRND";}
};

struct RERandom : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "RAND";}
};

struct REMultiply : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "MULT";}
};

struct REPulse : IRealElement
{
    AT_DECL_DNA_YAML
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "PULS";}
};

struct RETimeScale : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory dv;
    const char* ClassID() const {return "SCAL";}
};

struct RELifetimePercent : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory percent;
    const char* ClassID() const {return "RLPT";}
};

struct RESineWave : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory frequency;
    RealElementFactory amplitude;
    RealElementFactory phase;
    const char* ClassID() const {return "SINE";}
};

struct REInitialSwitch : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "ISWT";}
};

struct RECompareLessThan : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory ca;
    RealElementFactory cb;
    RealElementFactory pass;
    RealElementFactory fail;
    const char* ClassID() const {return "CLTN";}
};

struct RECompareEquals : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory ca;
    RealElementFactory cb;
    RealElementFactory pass;
    RealElementFactory fail;
    const char* ClassID() const {return "CEQL";}
};

struct REParticleAdvanceParam1 : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PAP1";}
};

struct REParticleAdvanceParam2 : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PAP2";}
};

struct REParticleAdvanceParam3 : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PAP3";}
};

struct REParticleAdvanceParam4 : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PAP4";}
};

struct REParticleAdvanceParam5 : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PAP5";}
};

struct REParticleAdvanceParam6 : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PAP6";}
};

struct REParticleAdvanceParam7 : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PAP7";}
};

struct REParticleAdvanceParam8 : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PAP8";}
};

struct REParticleSizeOrLineLength : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PSLL";}
};

struct REParticleRotationOrLineWidth : IRealElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PRLW";}
};

struct RESubtract : IRealElement
{
    AT_DECL_DNA_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "SUB_";}
};

struct REVectorMagnitude : IRealElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory vec;
    const char* ClassID() const {return "VMAG";}
};

struct REVectorXToReal : IRealElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory vec;
    const char* ClassID() const {return "VXTR";}
};

struct REVectorYToReal : IRealElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory vec;
    const char* ClassID() const {return "VYTR";}
};

struct REVectorZToReal : IRealElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory vec;
    const char* ClassID() const {return "VZTR";}
};

struct RECEXT : IRealElement
{
    AT_DECL_DNA_YAML
    IntElementFactory index;
    const char* ClassID() const {return "CEXT";}
};

struct REIntTimesReal : IRealElement
{
    AT_DECL_DNA_YAML
    IntElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "ITRL";}
};

struct IEKeyframeEmitter : IIntElement
{
    AT_DECL_DNA_YAML
    Value<atUint32> percentageTween;
    Value<atUint32> unk1;
    Value<bool> loop;
    Value<atUint8> unk2;
    Value<atUint32> loopEnd;
    Value<atUint32> loopStart;
    Value<atUint32> count;
    Vector<atUint32, AT_DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentageTween ? "KEYP" : "KEYE";}
};

struct IEDeath : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory passthrough;
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "DETH";}
};

struct IEClamp : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory min;
    IntElementFactory max;
    IntElementFactory val;
    const char* ClassID() const {return "CLMP";}
};

struct IETimeChain : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "CHAN";}
};

struct IEAdd : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "ADD_";}
};

struct IEConstant : IIntElement
{
    AT_DECL_EXPLICIT_DNA_YAML
    Value<atUint32> val;
    const char* ClassID() const {return "CNST";}
};

struct IEImpulse : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory val;
    const char* ClassID() const {return "IMPL";}
};

struct IELifetimePercent : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory percent;
    const char* ClassID() const {return "ILPT";}
};

struct IEInitialRandom : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "IRND";}
};

struct IEPulse : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "PULS";}
};

struct IEMultiply : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "MULT";}
};

struct IESampleAndHold : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory val;
    IntElementFactory waitMin;
    IntElementFactory waitMax;
    const char* ClassID() const {return "SPAH";}
};

struct IERandom : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "RAND";}
};

struct IETimeScale : IIntElement
{
    AT_DECL_DNA_YAML
    RealElementFactory dv;
    const char* ClassID() const {return "TSCL";}
};

struct IEGTCP : IIntElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "GTCP";}
};

struct IEModulo : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassID() const {return "MODU";}
};

struct IESubtract : IIntElement
{
    AT_DECL_DNA_YAML
    IntElementFactory direction;
    IntElementFactory baseRadius;
    const char* ClassID() const {return "SUB_";}
};

struct VECone : IVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory a;
    RealElementFactory b;
    const char* ClassID() const {return "CONE";}
};

struct VETimeChain : IVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "CHAN";}
};

struct VEAngleCone : IVectorElement
{
    AT_DECL_DNA_YAML
    RealElementFactory angleXBias;
    RealElementFactory angleYBias;
    RealElementFactory angleXRange;
    RealElementFactory angleYRange;
    RealElementFactory magnitude;
    const char* ClassID() const {return "ANGC";}
};

struct VEAdd : IVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    const char* ClassID() const {return "ADD_";}
};

struct VECircleCluster : IVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory circleOffset;
    VectorElementFactory circleNormal;
    IntElementFactory cycleFrames;
    RealElementFactory randomFactor;
    const char* ClassID() const {return "CCLU";}
};

struct VEConstant : IVectorElement
{
    AT_DECL_EXPLICIT_DNA_YAML
    RealElementFactory comps[3];
    const char* ClassID() const {return "CNST";}
};

struct VECircle : IVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory circleOffset;
    VectorElementFactory circleNormal;
    RealElementFactory angleConstant;
    RealElementFactory angleLinear;
    RealElementFactory circleRadius;
    const char* ClassID() const {return "CIRC";}
};

struct VEKeyframeEmitter : IVectorElement
{
    AT_DECL_DNA_YAML
    Value<atUint32> percentageTween;
    Value<atUint32> unk1;
    Value<bool> loop;
    Value<atUint8> unk2;
    Value<atUint32> loopEnd;
    Value<atUint32> loopStart;
    Value<atUint32> count;
    Vector<atVec3f, AT_DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentageTween ? "KEYP" : "KEYE";}
};

struct VEMultiply : IVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory a;
    VectorElementFactory b;
    const char* ClassID() const {return "MULT";}
};

struct VERealToVector : IVectorElement
{
    AT_DECL_DNA_YAML
    RealElementFactory a;
    const char* ClassID() const {return "RTOV";}
};

struct VEPulse : IVectorElement
{
    AT_DECL_DNA_YAML
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    VectorElementFactory a;
    VectorElementFactory b;
    const char* ClassID() const {return "PULS";}
};

struct VEParticleVelocity : IVectorElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PVEL";}
};

struct VESPOS : IVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory a;
    const char* ClassID() const {return "SPOS";}
};

struct VEPLCO : IVectorElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PLCO";}
};

struct VEPLOC : IVectorElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PLOC";}
};

struct VEPSOR : IVectorElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PSOR";}
};

struct VEPSOF : IVectorElement
{
    AT_DECL_DNA_YAML
    const char* ClassID() const {return "PSOF";}
};

struct CEKeyframeEmitter : IColorElement
{
    AT_DECL_DNA_YAML
    Value<atUint32> percentageTween;
    Value<atUint32> unk1;
    Value<bool> loop;
    Value<atUint8> unk2;
    Value<atUint32> loopEnd;
    Value<atUint32> loopStart;
    Value<atUint32> count;
    Vector<atVec4f, AT_DNA_COUNT(count)> keys;
    const char* ClassID() const {return percentageTween ? "KEYP" : "KEYE";}
};

struct CEConstant : IColorElement
{
    AT_DECL_EXPLICIT_DNA_YAML
    RealElementFactory comps[4];
    const char* ClassID() const {return "CNST";}
};

struct CETimeChain : IColorElement
{
    AT_DECL_DNA_YAML
    ColorElementFactory a;
    ColorElementFactory b;
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "CHAN";}
};

struct CEFadeEnd : IColorElement
{
    AT_DECL_DNA_YAML
    ColorElementFactory a;
    ColorElementFactory b;
    RealElementFactory startFrame;
    RealElementFactory endFrame;
    const char* ClassID() const {return "CFDE";}
};

struct CEFade : IColorElement
{
    AT_DECL_DNA_YAML
    ColorElementFactory a;
    ColorElementFactory b;
    RealElementFactory endFrame;
    const char* ClassID() const {return "FADE";}
};

struct CEPulse : IColorElement
{
    AT_DECL_DNA_YAML
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    ColorElementFactory a;
    ColorElementFactory b;
    const char* ClassID() const {return "PULS";}
};

struct MVEImplosion : IModVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory implodePoint;
    RealElementFactory velocityScale;
    RealElementFactory maxRadius;
    RealElementFactory minRadius;
    BoolHelper enableMinRadius;
    const char* ClassID() const {return "IMPL";}
};

struct MVEExponentialImplosion : IModVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory implodePoint;
    RealElementFactory velocityScale;
    RealElementFactory maxRadius;
    RealElementFactory minRadius;
    BoolHelper enableMinRadius;
    const char* ClassID() const {return "EMPL";}
};

struct MVETimeChain : IModVectorElement
{
    AT_DECL_DNA_YAML
    ModVectorElementFactory a;
    ModVectorElementFactory b;
    IntElementFactory thresholdFrame;
    const char* ClassID() const {return "CHAN";}
};

struct MVEBounce : IModVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory planePoint;
    VectorElementFactory planeNormal;
    RealElementFactory friction;
    RealElementFactory restitution;
    BoolHelper dieOnPenetrate;
    const char* ClassID() const {return "BNCE";}
};

struct MVEConstant : IModVectorElement
{
    AT_DECL_EXPLICIT_DNA_YAML
    RealElementFactory comps[3];
    const char* ClassID() const {return "CNST";}
};

struct MVEGravity : IModVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory acceleration;
    const char* ClassID() const {return "GRAV";}
};

struct MVEExplode : IModVectorElement
{
    AT_DECL_DNA_YAML
    RealElementFactory impulseMagnitude;
    RealElementFactory falloffFactor;
    const char* ClassID() const {return "EXPL";}
};

struct MVESetPosition : IModVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory position;
    const char* ClassID() const {return "SPOS";}
};

struct MVELinearImplosion : IModVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory implodePoint;
    RealElementFactory velocityScale;
    RealElementFactory maxRadius;
    RealElementFactory minRadius;
    BoolHelper enableMinRadius;
    const char* ClassID() const {return "LMPL";}
};

struct MVEPulse : IModVectorElement
{
    AT_DECL_DNA_YAML
    IntElementFactory aDuration;
    IntElementFactory bDuration;
    ModVectorElementFactory a;
    ModVectorElementFactory b;
    const char* ClassID() const {return "PULS";}
};

struct MVEWind : IModVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory windVelocity;
    RealElementFactory factor;
    const char* ClassID() const {return "WIND";}
};

struct MVESwirl : IModVectorElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory helixPoint;
    VectorElementFactory curveBinormal;
    RealElementFactory filterGain;
    RealElementFactory tangentialVelocity;
    const char* ClassID() const {return "SWRL";}
};

struct EESimpleEmitter : IEmitterElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory position;
    VectorElementFactory velocity;
    const char* ClassID() const {return "SEMR";}
};

struct VESphere : IEmitterElement
{
    AT_DECL_DNA_YAML
    VectorElementFactory sphereOrigin;
    RealElementFactory sphereRadius;
    RealElementFactory magnitude;
    const char* ClassID() const {return "SPHE";}
};

struct VEAngleSphere : IEmitterElement
{
    AT_DECL_DNA_YAML
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
    AT_DECL_EXPLICIT_DNA_YAML
    const char* ClassID() const {return "SETR";}
};

template <class IDType>
struct UVEConstant : IUVElement
{
    AT_DECL_EXPLICIT_DNA_YAML
    AT_SUBDECL_DNA
    CastIDToZero<IDType> tex;
    const char* ClassID() const {return "CNST";}

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(tex, pathsOut);
    }
};

template <class IDType>
struct UVEAnimTexture : IUVElement
{
    AT_DECL_EXPLICIT_DNA_YAML
    AT_SUBDECL_DNA
    CastIDToZero<IDType> tex;
    IntElementFactory tileW;
    IntElementFactory tileH;
    IntElementFactory strideW;
    IntElementFactory strideH;
    IntElementFactory cycleFrames;
    Value<bool> loop = false;
    const char* ClassID() const {return "ATEX";}

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(tex, pathsOut);
    }
};

template <class IDType>
struct UVElementFactory : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    AT_SUBDECL_DNA
    DNAFourCC m_type;
    std::unique_ptr<IUVElement> m_elem;
    operator bool() const {return m_elem.operator bool();}
};

template <class IDType>
struct SpawnSystemKeyframeData : BigDNA
{
    Value<atUint32> a;
    Value<atUint32> b;
    Value<atUint32> endFrame;
    Value<atUint32> d;

    struct SpawnSystemKeyframeInfo : BigDNA
    {
        IDType id;
        Value<atUint32> a;
        Value<atUint32> b;
        Value<atUint32> c;
        AT_DECL_EXPLICIT_DNA_YAML
    };

    std::vector<std::pair<atUint32, std::vector<SpawnSystemKeyframeInfo>>> spawns;

    AT_DECL_EXPLICIT_DNA_YAML
    AT_SUBDECL_DNA

    operator bool() const {return spawns.size() != 0;}

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        for (const auto& p : spawns)
            for (const SpawnSystemKeyframeInfo& info : p.second)
                g_curSpec->flattenDependencies(info.id, pathsOut);
    }
};

template <class IDType>
struct ChildResourceFactory : BigDNA
{
    IDType id;
    AT_DECL_EXPLICIT_DNA_YAML
    AT_SUBDECL_DNA
    operator bool() const {return id.operator bool();}
};

}

