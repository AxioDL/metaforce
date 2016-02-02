#ifndef __COMMON_PARTICLECOMMON_HPP__
#define __COMMON_PARTICLECOMMON_HPP__

#include "DNACommon.hpp"

namespace Retro
{
namespace DNAParticle
{

struct IElement : BigYAML
{
    Delete _d;
    virtual ~IElement() = default;
    virtual const char* ClassName() const=0;
};

struct IRealElement : IElement {Delete _d;};
struct RealElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IRealElement> m_elem;

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

    void read(Athena::io::YAMLDocReader& r);
    void write(Athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(Athena::io::IStreamReader& r);
    void write(Athena::io::IStreamWriter& w) const;
};

struct RELifetimeTween : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassName() const {return "LFTW";}
};

struct REConstant : IRealElement
{
    DECL_YAML
    Value<float> val;
    const char* ClassName() const {return "CNST";}
};

struct RETimeChain : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    IntElementFactory c;
    const char* ClassName() const {return "CHAN";}
};

struct REAdd : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassName() const {return "ADD_";}
};

struct REClamp : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    const char* ClassName() const {return "CLMP";}
};

struct REKeyframeEmitter : IRealElement
{
    DECL_YAML
    Value<atUint32> a;
    Value<atUint32> b;
    Value<atUint8> c;
    Value<atUint8> d;
    Value<atUint32> e;
    Value<atUint32> f;
    Value<atUint32> count;
    Vector<float, DNA_COUNT(count)> keys;
    const char* ClassName() const {return "KEYE";}
};

struct REInitialRandom : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassName() const {return "IRND";}
};

struct REKeyframeEmitterP : REKeyframeEmitter
{
    Delete _d;
    const char* ClassName() const {return "KEYP";}
};

struct RERandom : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassName() const {return "RAND";}
};

struct REMultiply : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    const char* ClassName() const {return "MULT";}
};

struct REPulse : IRealElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
    const char* ClassName() const {return "PULS";}
};

struct RETimeScale : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    const char* ClassName() const {return "SCAL";}
};

struct RELifetimePercent : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    const char* ClassName() const {return "RLPT";}
};

struct RESineWave : IRealElement
{
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    const char* ClassName() const {return "SINE";}
};

struct IEKeyframeEmitter : IIntElement
{
    DECL_YAML
    Value<atUint32> a;
    Value<atUint32> b;
    Value<atUint8> c;
    Value<atUint8> d;
    Value<atUint32> e;
    Value<atUint32> f;
    Value<atUint32> count;
    Vector<atUint32, DNA_COUNT(count)> keys;
    const char* ClassName() const {return "KEYE";}
};

struct IEDeath : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassName() const {return "DETH";}
};

struct IEClamp : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory c;
    const char* ClassName() const {return "CLMP";}
};

struct IETimeChain : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory c;
    const char* ClassName() const {return "CHAN";}
};

struct IEAdd : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassName() const {return "ADD_";}
};

struct IEConstant : IIntElement
{
    DECL_YAML
    Value<atUint32> val;
    const char* ClassName() const {return "CNST";}
};

struct IEImpulse : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    const char* ClassName() const {return "IMPL";}
};

struct IELifetimePercent : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    const char* ClassName() const {return "ILPT";}
};

struct IEInitialRandom : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassName() const {return "IRND";}
};

struct IEPulse : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory c;
    IntElementFactory d;
    const char* ClassName() const {return "PULS";}
};

struct IEMultiply : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassName() const {return "MULT";}
};

struct IEKeyframeEmitterP : IEKeyframeEmitter
{
    Delete _d;
    const char* ClassName() const {return "KEYP";}
};

struct IESampleAndHold : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    IntElementFactory c;
    const char* ClassName() const {return "SPAH";}
};

struct IERandom : IIntElement
{
    DECL_YAML
    IntElementFactory a;
    IntElementFactory b;
    const char* ClassName() const {return "RAND";}
};

struct IETimeScale : IIntElement
{
    DECL_YAML
    RealElementFactory a;
    const char* ClassName() const {return "TSCL";}
};


template <class IDType>
struct ChildGeneratorDesc : BigYAML
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
            return __isz + 8;
        else
            return __isz + 4;
    }
    void read(Athena::io::IStreamReader& r)
    {
        id.clear();
        if (r.readUint32Big() == 'CNST')
            id.read(r);
    }
    void write(Athena::io::IStreamWriter& w) const
    {
        if (id)
        {
            w.writeUint32Big('CNST');
            id.write(w);
        }
        else
            w.writeUint32Big('NONE');
    }
};

}
}

#endif // __COMMON_PARTICLECOMMON_HPP__
