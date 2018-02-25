#ifndef CVAR_HPP
#define CVAR_HPP

#include <string>
#include <functional>
#include <athena/Types.hpp>
#include <athena/Global.hpp>
#include <athena/DNAYaml.hpp>

namespace hecl
{
namespace DNACVAR
{
enum class EType : atUint8
{
    Boolean,
    Integer,
    Float,
    Literal,
    Vec4f
};

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
ENABLE_BITWISE_ENUM(EFlags)

class CVar : public athena::io::DNA<athena::Big>
{
public:
    AT_DECL_DNA
    String<-1>    m_name;
    String<-1>    m_value;
};

struct CVarContainer : public athena::io::DNA<athena::Big>
{
    AT_DECL_DNA
    Value<atUint32> magic = 'CVAR';
    Value<atUint32> cvarCount;
    Vector<CVar, AT_DNA_COUNT(cvarCount)> cvars;
};

}

class CVarManager;
class CVar : protected DNACVAR::CVar
{
    friend class CVarManager;
    Delete _d;

public:
    typedef std::function<void(CVar*)> ListenerFunc;

    using EType = DNACVAR::EType;
    using EFlags = DNACVAR::EFlags;

    CVar(std::string_view name, std::string_view value, std::string_view help, EType type, EFlags flags, CVarManager& parent);
    CVar(std::string_view name, std::string_view value, std::string_view help, EFlags flags, CVarManager& parent);
    CVar(std::string_view name, float value, std::string_view help, EFlags flags, CVarManager& parent);
    CVar(std::string_view name, bool  value, std::string_view help, EFlags flags, CVarManager& parent);
    CVar(std::string_view name, int   value, std::string_view help, EFlags flags, CVarManager& parent);
    CVar(std::string_view name, const atVec4f& value, std::string_view help, EFlags flags, CVarManager& parent);


    std::string_view name() const { return m_name; }
    std::string_view rawHelp() const { return m_help; }
    std::string help() const;
    std::string value() const { return m_value; }

    atVec4f toVec4f(bool* isValid = nullptr) const;
    float toFloat(bool* isValid = nullptr) const;
    bool  toBoolean(bool* isValid = nullptr) const;
    int   toInteger(bool* isValid = nullptr) const;
    const std::wstring toWideLiteral(bool* isValid = nullptr) const;
    const std::string toLiteral(bool* isValid = nullptr) const;

    bool fromVec4f(const atVec4f& val);
    bool fromFloat(float val);
    bool fromBoolean(bool val);
    bool fromInteger(int val);
    bool fromLiteral(std::string_view val);
    bool fromLiteral(std::wstring_view val);
    bool fromLiteralToType(std::string_view val, bool setDefault = false);
    bool fromLiteralToType(std::wstring_view val, bool setDefault = false);

    bool isFloat()    const { return m_type == EType::Float; }
    bool isBoolean()  const { return m_type == EType::Boolean; }
    bool isInteger()  const { return m_type == EType::Integer; }
    bool isLiteral()  const { return m_type == EType::Literal; }
    bool isVec4f()    const { return m_type == EType::Vec4f; }
    bool isModified() const;
    bool modificationRequiresRestart() const;
    bool isReadOnly() const;
    bool isCheat()    const;
    bool isHidden()   const;
    bool isArchive()  const;
    bool isInternalArchivable() const;
    bool wasDeserialized() const;
    bool hasDefaultValue() const;
    void clearModified();
    void setModified();

    EType type() const  { return m_type; }
    EFlags flags() const  { return m_flags; }

    /*!
     * \brief Unlocks the CVar for writing if it is ReadOnly.
     * <b>Handle with care!!!</b> if you use unlock(), make sure
     * you lock the cvar using lock()
     * \see lock
     */
    void unlock();

    /*!
     * \brief Locks the CVar to prevent writing if it is ReadOnly.
     * Unlike its partner function unlock, lock is harmless
     * \see unlock
     */
    void lock();

    void addListener(ListenerFunc func) { m_listeners.push_back(func); }

private:
    void dispatch();
    EType  m_type;
    std::string m_help;
    std::string m_defaultValue;
    EFlags      m_flags;
    EFlags      m_oldFlags;
    bool        m_unlocked = false;
    bool        m_wasDeserialized = false;

    CVarManager& m_mgr;

    std::vector<ListenerFunc> m_listeners;
};

class CVarUnlocker
{
    CVar* m_cvar;
public:
    CVarUnlocker(CVar* cvar) : m_cvar(cvar) { if (m_cvar) m_cvar->unlock(); }
    ~CVarUnlocker() { if (m_cvar) m_cvar->lock(); }
};

}
#endif // CVAR_HPP

