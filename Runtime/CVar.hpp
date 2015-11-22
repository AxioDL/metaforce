#ifndef CVAR_HPP
#define CVAR_HPP

#include <string>
#include <functional>
#include "CColor.hpp"
#include "DataSpec/DNACommon/CVar.hpp"

namespace Retro
{

class CVarManager;
class CVar : protected DNACVAR::CVar
{
    friend class CVarManager;

public:
    typedef std::function<void(CVar*)> ListenerFunc;

    using EType = DNACVAR::EType;
    using EFlags = DNACVAR::EFlags;

    CVar(const std::string& name, const std::string& value, const std::string& help, EType type, EFlags flags, CVarManager& parent);
    CVar(const std::string& name, const std::string& value, const std::string& help, EFlags flags, CVarManager& parent);
    CVar(const std::string& name, const Zeus::CColor& value, const std::string& help, EFlags flags, CVarManager& parent);
    CVar(const std::string& name, float value, const std::string& help, EFlags flags, CVarManager& parent);
    CVar(const std::string& name, bool  value, const std::string& help, EFlags flags, CVarManager& parent);
    CVar(const std::string& name, int   value, const std::string& help, EFlags flags, CVarManager& parent);


    std::string  name() const;
    std::string  help() const;

    Zeus::CColor toColor(bool* isValid = nullptr) const;
    float toFloat(bool* isValid = nullptr) const;
    bool  toBoolean(bool* isValid = nullptr) const;
    int   toInteger(bool* isValid = nullptr) const;
    const std::wstring toWideLiteral(bool* isValid = nullptr) const;
    const std::string toLiteral(bool* isValid = nullptr) const;

    bool fromColor(const Zeus::CColor& val);
    bool fromFloat(float val);
    bool fromBoolean(bool val);
    bool fromInteger(int val);
    bool fromLiteral(const std::string& val);
    bool fromLiteral(const std::wstring& val);

    bool isFloat()    const { return m_type == EType::Float; }
    bool isBoolean()  const { return m_type == EType::Boolean; }
    bool isInteger()  const { return m_type == EType::Integer; }
    bool isLiteral()  const { return m_type == EType::Literal; }
    bool isColor()    const { return m_type == EType::Color; }
    bool isModified() const;
    bool isReadOnly() const;
    bool isCheat()    const;
    bool isHidden()   const;
    bool isArchive()  const;
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
     * Unlike it's partner function unlock, lock is harmless
     * \see unlock
     */
    void lock();

    void addListener(ListenerFunc func) { m_listeners.push_back(func); }

private:
    void dispatch();
    std::string m_help;
    std::string m_defaultValue;
    EFlags      m_flags;
    bool        m_allowedWrite;

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

