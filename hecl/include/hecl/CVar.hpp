#ifndef CVAR_HPP
#define CVAR_HPP

#include <string>
#include <functional>
#include <athena/Types.hpp>
#include <athena/Global.hpp>
#include <athena/DNAYaml.hpp>

namespace hecl
{
class CVar
{
    friend class CVarManager;
public:
    enum EFlags
    {
        System             = (1 << 0),
        Game               = (1 << 1),
        Editor             = (1 << 2),
        Gui                = (1 << 3),
        Cheat              = (1 << 4),
        Hidden             = (1 << 5),
        ReadOnly           = (1 << 6),
        Archive            = (1 << 7),
        InternalArchivable = (1 << 8),
        Modified           = (1 << 9),
        ModifyRestart      = (1 << 10) /*!< If this bit is set, any modification will inform the user that a restart is required */
    };
protected:
    std::string m_name;
    std::string m_help;
    EFlags m_flags;
    EFlags m_oldFlags;
    bool m_wasDeserialized = false;
    bool m_unlocked = false;

    virtual bool _fromString(std::string_view sv) = 0;
public:
    CVar(std::string_view name, std::string_view help, EFlags flags);
    virtual ~CVar() = default;
    std::string name() const { return m_name; }
    std::string help() const;
    std::string rawHelp() const { return m_help; }
    virtual std::string toString() const=0;
    bool fromString(std::string_view v);
    virtual void deserialize(athena::io::YAMLDocReader& reader) = 0;
    virtual void serialize(athena::io::YAMLDocWriter& writer) const = 0;
    virtual bool hasDefaultValue() const = 0;
    virtual std::string defaultValueString() const = 0;

    bool isModified() const;
    bool modificationRequiresRestart() const;
    bool isReadOnly() const;
    bool isCheat()    const;
    bool isHidden()   const;
    bool isArchive()  const;
    bool isInternalArchivable() const;
    bool wasDeserialized() const;
    void clearModified();
    void setModified();

    EFlags flags() const  { return m_flags; }
    void unlock();
    void lock();
};
ENABLE_BITWISE_ENUM(CVar::EFlags)


template<typename T>
class TCVar : public CVar
{
    friend class CVarManager;
protected:
    T& m_value;
    T m_defaultValue;
    bool _fromString(std::string_view sv);
public:
    TCVar(T& value, std::string_view name, std::string_view help, EFlags flags);

    virtual std::string toString() const;
    std::wstring toWideString() const;
    virtual void deserialize(athena::io::YAMLDocReader& reader);
    virtual void serialize(athena::io::YAMLDocWriter& writer) const;
    virtual bool hasDefaultValue() const;

    T value() const;
    T defaultValue() const;
    std::string defaultValueString() const;
};

class CVarUnlocker
{
    CVar* m_cvar;
public:
    CVarUnlocker(CVar* cvar) : m_cvar(cvar) { if (m_cvar) m_cvar->unlock(); }
    ~CVarUnlocker() { if (m_cvar) m_cvar->lock(); }
};

class Vec3fCVar : public CVar
{
    friend class CVarManager;
    atVec3f& m_value;
    atVec3f m_defaultValue;
    bool _fromString(std::string_view v);
public:
    Vec3fCVar(atVec3f& value, std::string_view name, std::string_view help, EFlags flags);

    std::string toString() const;
    bool hasDefaultValue() const;
    void deserialize(athena::io::YAMLDocReader& reader);
    void serialize(athena::io::YAMLDocWriter& writer) const;
    atVec3f value() const { return m_value; }
    atVec3f defaultValue() const { return m_defaultValue; }

    std::string defaultValueString() const;
};

class Vec3dCVar : public CVar
{
    friend class CVarManager;
    atVec3d& m_value;
    atVec3d m_defaultValue;
    bool _fromString(std::string_view v);
public:
    Vec3dCVar(atVec3d& value, std::string_view name, std::string_view help, EFlags flags);

    std::string toString() const;
    bool hasDefaultValue() const;
    void deserialize(athena::io::YAMLDocReader& reader);
    void serialize(athena::io::YAMLDocWriter& writer) const;
    atVec3d value() const { return m_value; }
    atVec3d defaultValue() const { return m_defaultValue; }
    std::string defaultValueString() const;
};

class Vec4fCVar : public CVar
{
    friend class CVarManager;
    atVec4f& m_value;
    atVec4f m_defaultValue;
    bool _fromString(std::string_view v);
public:
    Vec4fCVar(atVec4f& value, std::string_view name, std::string_view help, EFlags flags);
    virtual ~Vec4fCVar() = default;

    std::string toString() const;
    bool hasDefaultValue() const;
    void deserialize(athena::io::YAMLDocReader& reader);
    void serialize(athena::io::YAMLDocWriter& writer) const;
    atVec4f value() const { return m_value; }
    atVec4f defaultValue() const { return m_defaultValue; }
    std::string defaultValueString() const;
};

class Vec4dCVar : public CVar
{
    friend class CVarManager;
    atVec4d& m_value;
    atVec4d m_defaultValue;
    bool _fromString(std::string_view v);
public:
    Vec4dCVar(atVec4d& value, std::string_view name, std::string_view help, EFlags flags);

    std::string toString() const;
    bool hasDefaultValue() const;
    void deserialize(athena::io::YAMLDocReader& reader);
    void serialize(athena::io::YAMLDocWriter& writer) const;
    atVec4d value() const { return m_value; }
    atVec4d defaultValue() const { return m_defaultValue; }
    std::string defaultValueString() const;
};

class StringCVar : public CVar
{
    friend class CVarManager;
    std::string& m_value;
    std::string m_defaultValue;
    bool _fromString(std::string_view v);
public:
    StringCVar(std::string& value, std::string_view name, std::string_view help, EFlags flags);

    std::string toString() const;
    bool hasDefaultValue() const;
    void deserialize(athena::io::YAMLDocReader& reader);
    void serialize(athena::io::YAMLDocWriter& writer) const;
    const std::string& value() const { return m_value; }
    const std::string& defaultValue() const { return m_defaultValue; }
    std::string defaultValueString() const { return defaultValue(); }
};

using BoolCVar = TCVar<bool>;
using Int16CVar = TCVar<int16_t>;
using Uint16CVar = TCVar<uint16_t>;
using Int32CVar = TCVar<int32_t>;
using Uint32CVar = TCVar<uint32_t>;
using Int64CVar = TCVar<int64_t>;
using Uint64CVar = TCVar<uint64_t>;
using FloatCVar = TCVar<float>;
using DoubleCVar = TCVar<double>;
}
#endif // CVAR_HPP

