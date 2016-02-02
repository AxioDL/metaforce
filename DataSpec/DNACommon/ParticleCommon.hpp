#ifndef __COMMON_PARTICLECOMMON_HPP__
#define __COMMON_PARTICLECOMMON_HPP__

#include "DNACommon.hpp"

namespace Retro
{
namespace DNAParticle
{

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

struct IVectorElement : IElement {Delete _d;};
struct VectorElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IVectorElement> m_elem;

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
    DECL_YAML
    Value<float> val;
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
    DECL_YAML
    Value<atUint32> val;
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
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
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
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
    RealElementFactory d;
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
    DECL_YAML
    RealElementFactory a;
    RealElementFactory b;
    RealElementFactory c;
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
            return __isz + 8;
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
            __isz += 4;
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
};

template <class IDType>
struct UVElementFactory : BigYAML
{
    Delete _d;
    std::unique_ptr<IUVElement> m_elem;

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
};

}
}

#endif // __COMMON_PARTICLECOMMON_HPP__
