#include "CVar.hpp"
#include "CVarManager.hpp"
#include "CVarManager.hpp"
#include "CBasics.hpp"

#include <Athena/Utility.hpp>

namespace Retro
{
CVar::CVar(const std::string& name, const std::string &value, const std::string &help, EType type, EFlags flags, CVarManager* parent)
{
    m_name= name;
    m_value = value;
    m_defaultValue = value;
    m_help = help;
    m_type = type;
    m_flags = flags;
    m_allowedWrite = false;
    m_mgr = parent;
}

CVar::CVar(const std::string& name, const std::string& value, const std::string& help, CVar::EFlags flags, CVarManager* parent)
{
    // Unlock the cvar for writing if readonly
    unlock();

    m_name= name;
    m_help = help;
    m_type = EType::Literal;
    m_flags = flags;
    m_allowedWrite = false;
    m_mgr = parent;

    fromLiteral(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

CVar::CVar(const std::string& name, const Zeus::CColor& value, const std::string& help, EFlags flags, CVarManager* parent)
{
    // Unlock the cvar for writing if readonly
    unlock();

    m_name= name;
    m_help = help;
    m_type = EType::Color;
    m_flags = flags;
    m_allowedWrite = false;
    m_mgr = parent;

    fromColor(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

CVar::CVar(const std::string& name, float value, const std::string& help, EFlags flags, CVarManager* parent)
{
    // Unlock the cvar for writing if readonly
    unlock();

    m_name= name;
    m_help = help;
    m_type = EType::Float;
    m_flags = flags;
    m_allowedWrite = false;
    m_mgr = parent;

    fromFloat(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

CVar::CVar(const std::string& name, bool value, const std::string& help, CVar::EFlags flags, CVarManager* parent)
{
    // Unlock the cvar for writing if readonly
    unlock();

    m_name= name;
    m_help = help;
    m_type = EType::Boolean;
    m_flags = flags;
    m_allowedWrite = false;
    m_mgr = parent;

    fromBoolean(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

CVar::CVar(const std::string& name, int value, const std::string& help, CVar::EFlags flags, CVarManager* parent)
{
    // Unlock the cvar for writing if readonly
    unlock();

    m_name= name;
    m_help = help;
    m_type = EType::Integer;
    m_flags = flags;
    m_allowedWrite = false;
    m_mgr = parent;

    fromInteger(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

std::string CVar::name() const
{
    return m_name;
}

std::string CVar::help() const
{
    return std::string(m_help + (m_defaultValue != std::string() ? "\ndefault: " + m_defaultValue : "") +
                       (isReadOnly() ? "[ReadOnly]" : ""));
}

Zeus::CColor CVar::toColor(bool* isValid) const
{
    if (m_type != EType::Color)
    {
        if (isValid != nullptr)
            *isValid = false;

        return Zeus::CColor();
    }

    if (isValid != NULL)
        *isValid = true;

    int r, g, b, a;
    std::sscanf(m_value.c_str(), "%i %i %i %i", &r, &g, &b, &a);

    return Zeus::CColor(Zeus::Comp8(r), Zeus::Comp8(g), Zeus::Comp8(b), Zeus::Comp8(a));
}

float CVar::toFloat(bool* isValid) const
{
    if (m_type != EType::Float)
    {
        if (isValid)
            *isValid = false;
        return 0.0f;
    }

    return strtof(m_value.c_str(), nullptr);
}

bool CVar::toBoolean(bool* isValid) const
{
    if (m_type != EType::Boolean)
    {
        if (isValid)
            *isValid = false;

        return false;
    }

    // We don't want to modify the original value;
    std::string tmp = m_value;
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

    if (!tmp.compare("yes") || !tmp.compare("true") || !tmp.compare("1"))
    {
        if (isValid)
            *isValid = true;
        return true;
    }
    else if (!tmp.compare("no") || !tmp.compare("false") || !tmp.compare("0"))
    {
        if (isValid)
            *isValid = true;
        return false;
    }

    if (isValid)
        *isValid = false;

    return false;
}

int CVar::toInteger(bool* isValid) const
{
    if (m_type != EType::Integer)
    {
        if (isValid)
            *isValid = false;
        return 0;
    }

    return strtol(m_value.c_str(), nullptr, 0);
}


const std::string& CVar::toLiteral(bool* isValid) const
{
    if (m_type != EType::Literal  /*&& (com_developer && com_developer->toBoolean())*/)
    {
        if (isValid != nullptr)
            *isValid = false;
    }
    else if (isValid != nullptr)
        *isValid = true;

    // Even if it's not a literal, it's still safe to return
    return m_value;
}

bool CVar::fromColor(const Zeus::CColor& val)
{
    if (isCheat())
        return false;

    if (m_type != EType::Color)
        return false;

    if (isReadOnly())
        return false;

    m_value.assign(CBasics::Stringize("%i %i %i %i", unsigned(val.r), unsigned(val.g), unsigned(val.b), unsigned(val.a)));
    m_flags |= EFlags::Modified;
    return true;
}

bool CVar::fromFloat(float val)
{
    if (isCheat() /*&& (!com_developer->toBoolean() && !com_enableCheats->toBoolean())*/)
        return false;

    if (m_type != EType::Float)
        return false;

    if (isReadOnly() /*&& (com_developer && !com_developer->toBoolean())*/)
        return false;

    m_value.assign(CBasics::Stringize("%f", val));
    setModified();
    return true;
}

bool CVar::fromBoolean(bool val)
{
    if (isCheat() /*&& (!com_developer->toBoolean() && !com_enableCheats->toBoolean())*/)
        return false;

    if (m_type != EType::Boolean)
        return false;

    if (isReadOnly() /*&& (com_developer && !com_developer->toBoolean())*/)
        return false;

    if (val)
        m_value = "true";
    else
        m_value = "false";

    setModified();
    return true;
}

bool CVar::fromInteger(int val)
{
    if (isCheat() /*&& (!com_developer->toBoolean() && !com_enableCheats->toBoolean())*/)
        return false;

    if (m_type != EType::Integer)
        return false;

    if (isReadOnly() /*&& (com_developer && !com_developer->toBoolean())*/)
        return false;

    m_value = CBasics::Stringize("%i", val);
    setModified();
    return true;
}

bool CVar::fromLiteral(const std::string& val)
{
    if (isCheat() /*&& (!com_developer->toBoolean() && !com_enableCheats->toBoolean())*/)
        return false;

    if (m_type != EType::Literal)
        return false;

    if (isReadOnly() /*&& (com_developer && !com_developer->toBoolean())*/)
        return false;

    m_value.assign(val);
    setModified();
    return true;
}

bool CVar::isModified() const { return int(m_flags & EFlags::Modified) != 0;}

bool CVar::isReadOnly() const { return int(m_flags & EFlags::ReadOnly) != 0; }

bool CVar::isCheat() const { return int(m_flags & EFlags::Cheat) != 0; }

bool CVar::isHidden() const { return int(m_flags & EFlags::Hidden) != 0; }

bool CVar::isArchive() const { return int(m_flags & EFlags::Archive) != 0; }

void CVar::clearModified() { m_flags &= ~EFlags::Modified; }

void CVar::setModified() { m_flags |= EFlags::Modified; }

void CVar::unlock()
{
    if (!isReadOnly())
        return;

    if (!m_allowedWrite)
    {
        m_allowedWrite = true;
        m_flags &= ~EFlags::ReadOnly;
    }
}

void CVar::lock()
{
    if (!isReadOnly())
        return;

    if (m_allowedWrite)
    {
        m_flags |= EFlags::ReadOnly;
        m_allowedWrite = false;
        clearModified();
    }
}
}

