#include "HECL/HECL.hpp"
#include "HECL/CVar.hpp"
#include "HECL/CVarManager.hpp"

#include <Athena/Utility.hpp>
#include <algorithm>

namespace HECL
{
extern CVar* com_developer;
extern CVar* com_enableCheats;

CVar::CVar(const std::string& name, const std::string &value, const std::string &help, EType type, EFlags flags, CVarManager& parent)
    : m_mgr(parent)
{
    m_name= name;
    m_value = value;
    m_defaultValue = value;
    m_help = help;
    m_type = type;
    m_flags = flags;
    m_allowedWrite = false;
}

CVar::CVar(const std::string& name, const std::string& value, const std::string& help, CVar::EFlags flags, CVarManager& parent)
: m_mgr(parent)
{
    m_flags = flags;
    m_allowedWrite = false;
    m_name = name;
    m_help = help;
    m_type = EType::Literal;

    // Unlock the cvar for writing if readonly
    unlock();

    fromLiteral(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

CVar::CVar(const std::string& name, const atVec4f& value, const std::string& help, EFlags flags, CVarManager& parent)
    : m_mgr(parent)
{
    m_name= name;
    m_help = help;
    m_type = EType::Vec4f;
    m_flags = flags;
    m_allowedWrite = false;

    // Unlock the cvar for writing if readonly
    unlock();

    fromVec4f(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

CVar::CVar(const std::string& name, float value, const std::string& help, EFlags flags, CVarManager& parent)
    : m_mgr(parent)
{
    m_name= name;
    m_help = help;
    m_type = EType::Float;
    m_flags = flags;
    m_allowedWrite = false;

    // Unlock the cvar for writing if readonly
    unlock();

    fromFloat(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

CVar::CVar(const std::string& name, bool value, const std::string& help, CVar::EFlags flags, CVarManager& parent)
    : m_mgr(parent)
{
    m_name= name;
    m_help = help;
    m_type = EType::Boolean;
    m_flags = flags;
    m_allowedWrite = false;

    // Unlock the cvar for writing if readonly
    unlock();

    fromBoolean(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

CVar::CVar(const std::string& name, int value, const std::string& help, CVar::EFlags flags, CVarManager& parent)
    : m_mgr(parent)
{
    m_name= name;
    m_help = help;
    m_type = EType::Integer;
    m_flags = flags;
    m_allowedWrite = false;

    // Unlock the cvar for writing if readonly
    unlock();

    fromInteger(value);
    m_defaultValue = m_value;

    // Lock the cvar
    lock();
    // Clear the modified flag, just incase lock didn't do it.
    m_flags = flags;
}

std::string CVar::help() const
{
    return std::string(m_help + (m_defaultValue != std::string() ? "\ndefault: " + m_defaultValue : "") +
                       (isReadOnly() ? "[ReadOnly]" : ""));
}

atVec4f CVar::toVec4f(bool* isValid) const
{
    if (m_type != EType::Vec4f)
    {
        if (isValid != nullptr)
            *isValid = false;

        return atVec4f{};
    }

    if (isValid != NULL)
        *isValid = true;

    atVec4f vec;
    std::sscanf(m_value.c_str(), "%f %f %f %f", &vec.vec[0], &vec.vec[1], &vec.vec[2], &vec.vec[3]);

    return vec;
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

const std::string CVar::toLiteral(bool* isValid) const
{
    if (m_type != EType::Literal && (com_developer && com_developer->toBoolean()))
    {
        if (isValid != nullptr)
            *isValid = false;
    }
    else if (isValid != nullptr)
        *isValid = true;

    // Even if it's not a literal, it's still safe to return
    return m_value;
}

const std::wstring CVar::toWideLiteral(bool* isValid) const
{
    if (m_type != EType::Literal && (com_developer && com_developer->toBoolean()))
    {
        if (isValid != nullptr)
            *isValid = false;
    }
    else if (isValid != nullptr)
        *isValid = true;

    // Even if it's not a literal, it's still safe to return
    return HECL::UTF8ToWide(m_value);
}

bool CVar::fromVec4f(const atVec4f& val)
{
    if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
        return false;
    else if (isCheat())
        return false;

    if (m_type != EType::Vec4f)
        return false;

    if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
        return false;

    m_value.assign(HECL::Format("%f %f %f %f", val.vec[0], val.vec[1], val.vec[2], val.vec[3]));
    m_flags |= EFlags::Modified;
    return true;
}

bool CVar::fromFloat(float val)
{
    if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
        return false;
    else if (isCheat())
        return false;

    if (m_type != EType::Float)
        return false;

    if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
        return false;

    m_value.assign(HECL::Format("%f", val));
    setModified();
    return true;
}

bool CVar::fromBoolean(bool val)
{
    if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
        return false;
    else if (isCheat())
        return false;

    if (m_type != EType::Boolean)
        return false;

    if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
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
    if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
        return false;
    else if (isCheat())
        return false;

    if (m_type != EType::Integer)
        return false;

    if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
        return false;

    m_value = HECL::Format("%i", val);
    setModified();
    return true;
}

bool CVar::fromLiteral(const std::string& val)
{
    if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
        return false;
    else if (isCheat())
        return false;

    if (m_type != EType::Literal)
        return false;

    if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
        return false;

    m_value.assign(val);
    setModified();
    return true;
}

bool CVar::fromLiteral(const std::wstring& val)
{
    if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
        return false;
    else if (isCheat())
        return false;

    if (m_type != EType::Literal)
        return false;

    if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
        return false;

    m_value.assign(HECL::WideToUTF8(val));
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

void CVar::dispatch()
{
    for (const ListenerFunc& listen : m_listeners)
        listen(this);
}
}

