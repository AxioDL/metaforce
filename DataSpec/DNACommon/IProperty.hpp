#ifndef _DNACOMMON_IPROPERTY_HPP_
#define _DNACOMMON_IPROPERTY_HPP_

#include <memory>
#include "DNACommon.hpp"

namespace Retro
{
namespace DNACommon
{

class IPropertyBase: public BigYAML
{
protected:
    const UniqueID32 m_hash;
    const char* m_name;
    const char* m_yamlName;
public:
    IPropertyBase(UniqueID32 hash, const char* name) : m_hash(hash), m_name(name)
    { m_yamlName = (std::string(m_name) + "_" + m_hash.toString()).c_str(); }

    const UniqueID32 id() const { return m_hash; }

    const char* name() const { return m_name; }
    const char* yamlName() const { return m_yamlName; }
    static const char* DNAType() { return "Retro::DNACommon::IPropertyBase"; }
};

template <class ValueType>
class IProperty : public IPropertyBase
{
    Delete _d;
    ValueType m_val;
public:
    IProperty(UniqueID32 hash, const char* name)  : IPropertyBase(hash, name) { }

    ValueType value() const
    { return m_val; }

    void read(Athena::io::IStreamReader& in)         { m_val = in.readValBig<ValueType>(); }
    void read(Athena::io::YAMLDocReader& in)         { m_val = in.readVal<ValueType>(m_yamlName); }
    void write(Athena::io::IStreamWriter& out) const { out.writeVal(m_val); }
    void write(Athena::io::YAMLDocWriter& out) const { out.writeVal(m_yamlName, m_val, sizeof(ValueType)); }
    size_t binarySize(size_t __isz) const { return __isz + sizeof(ValueType); }

    IProperty& operator =(const ValueType& v) { m_val = v; return *this; }
    operator ValueType() { return m_val; }
};

using Int32Property   = IProperty<atInt32>;
using FloatProperty   = IProperty<float>;
using Vector3Property = IProperty<atVec3f>;
using ColorProperty   = IProperty<atVec4f>;
using BoolProperty    = IProperty<bool>;
using StringProperty  = IProperty<std::string>;

template <class IDType>
class UniqueIDProperty : public IPropertyBase
{
    IDType m_uniqueID;
    FourCC m_type;
public:
    UniqueIDProperty(FourCC type, UniqueID32 hash, const char* name) : IPropertyBase(hash, name), m_type(type) {}

    virtual void read(Athena::io::IStreamReader& in) { m_uniqueID.read(in); }

    virtual void read(Athena::io::YAMLDocReader& in)
    {
        in.enterSubRecord(m_yamlName);
        m_uniqueID.read(in);
        in.leaveSubRecord();
    }

    void write(Athena::io::IStreamWriter& out) const { m_uniqueID.write(out); }

    void write(Athena::io::YAMLDocWriter& out) const
    {
        out.enterSubRecord(m_yamlName);
        m_uniqueID.write(out);
        out.leaveSubRecord();
    }

    size_t binarySize(size_t __isz) { return __isz + IDType::BinarySize(); }

    UniqueIDProperty& operator =(const IDType& v) { m_uniqueID = v; return *this; }
    operator IDType() { return m_uniqueID; }
};

using UniqueID32Property  = UniqueIDProperty<UniqueID32 >;
using UniqueID64Property  = UniqueIDProperty<UniqueID64 >;
using UniqueID128Property = UniqueIDProperty<UniqueID128>;

}
}

#endif
