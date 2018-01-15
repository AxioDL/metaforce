#include "hecl/hecl.hpp"
#include "hecl/CVar.hpp"

#include <athena/Utility.hpp>
#include <algorithm>
#include <sstream>

namespace hecl
{
using namespace std::literals;
extern BoolCVar* com_developer;
extern BoolCVar* com_enableCheats;

CVar::CVar(std::string_view name, std::string_view help, CVar::EFlags flags)
    : m_name(name)
    , m_help(help)
    , m_flags(flags)
{
}

std::string CVar::help() const
{
    return std::string(m_help + (hasDefaultValue() ? " (default: " + defaultValueString() : "") +
                       (isReadOnly() ? " [ReadOnly]" : "")) + ")";
}

bool CVar::fromString(std::string_view v)
{
    if (isCheat() && (com_developer && !com_developer->value() && !com_enableCheats->value()))
        return false;
    else if (isCheat())
        return false;

    if (isReadOnly() && (com_developer && !com_developer->value()))
        return false;

    bool ret = _fromString(v);
    if (ret)
        m_flags |= EFlags::Modified;
    return ret;
}

bool CVar::isModified() const { return int(m_flags & EFlags::Modified) != 0; }
bool CVar::modificationRequiresRestart() const { return int(m_flags & EFlags::ModifyRestart) != 0; }

bool CVar::isReadOnly() const { return int(m_flags & EFlags::ReadOnly) != 0; }

bool CVar::isCheat() const { return int(m_flags & EFlags::Cheat) != 0; }

bool CVar::isHidden() const { return int(m_flags & EFlags::Hidden) != 0; }

bool CVar::isArchive() const { return int(m_flags & EFlags::Archive) != 0; }

bool CVar::isInternalArchivable() const { return int(m_flags & EFlags::InternalArchivable) != 0; }

bool CVar::wasDeserialized() const { return m_wasDeserialized; }

void CVar::clearModified()
{
    if (!modificationRequiresRestart())
        m_flags &= ~EFlags::Modified;
}

void CVar::setModified() { m_flags |= EFlags::Modified; }

void CVar::unlock()
{
    if (isReadOnly() && !m_unlocked)
    {
        m_oldFlags = m_flags;
        m_flags &= ~EFlags::ReadOnly;
        m_unlocked = true;
    }
}

void CVar::lock()
{
    if (!isReadOnly() && m_unlocked)
    {
        m_flags = m_oldFlags;
        m_unlocked = false;
    }
}

template <typename T>
TCVar<T>::TCVar(T& value, std::string_view name, std::string_view description, EFlags flags)
    : CVar(name, description, flags)
    , m_value(value)
    , m_defaultValue(value)
{
}

template<typename T>
std::string TCVar<T>::toString() const
{
    std::stringstream ss;
    ss << std::boolalpha << m_value;
    return ss.str();
}

template<typename T>
std::wstring TCVar<T>::toWideString() const
{
    return hecl::UTF8ToWide(toString());
}

template<typename T>
bool TCVar<T>::_fromString(std::string_view v)
{
    T tmp;
    std::stringstream ss;
    ss << std::boolalpha << v;
    ss >> tmp;
    if (ss.good())
        m_value = tmp;

    return ss.good();
}

template<typename T>
void TCVar<T>::deserialize(athena::io::YAMLDocReader &reader)
{
    m_value = reader.readVal<T>(m_name.c_str());
}

template<typename T>
void TCVar<T>::serialize(athena::io::YAMLDocWriter& writer) const
{
    writer.writeVal(m_name.c_str(), m_value);
}

template<typename T>
bool TCVar<T>::hasDefaultValue() const
{
    return m_value == m_defaultValue;
}


template<typename T>
T TCVar<T>::value() const
{
    return m_value;
}

template<typename T>
T TCVar<T>::defaultValue() const
{
    return m_defaultValue;
}

template<typename T>
std::string TCVar<T>::defaultValueString() const
{
    std::stringstream ss;
    ss << std::boolalpha << m_defaultValue;
    return ss.str();
}

Vec3fCVar::Vec3fCVar(atVec3f &value, std::string_view name, std::string_view description, CVar::EFlags flags)
    : CVar(name, description, flags)
    , m_value(value)
    , m_defaultValue(value)
{}

std::string Vec3fCVar::toString() const
{
    return athena::utility::sprintf("%f %f %f", double(m_value.vec[0]), double(m_value.vec[1]), double(m_value.vec[2]));
}

bool Vec3fCVar::_fromString(std::string_view v)
{
    float x, y, z;
    if (std::sscanf(v.data(), "%f %f %f", &x, &y, &z) != 3)
        return false;
    m_value.vec[0] = x;
    m_value.vec[1] = y;
    m_value.vec[2] = y;
    return true;
}

bool Vec3fCVar::hasDefaultValue() const
{
    return !memcmp(&m_value.vec, &m_defaultValue.vec[0], sizeof(atVec3f));
}

void Vec3fCVar::deserialize(athena::io::YAMLDocReader& reader)
{
    m_value = reader.readVec3f(m_name.c_str());
}

void Vec3fCVar::serialize(athena::io::YAMLDocWriter& writer) const
{
    writer.writeVec3f(m_name.c_str(), m_value);
}

std::string Vec3fCVar::defaultValueString() const
{
    return athena::utility::sprintf("%f %f %f", double(m_defaultValue.vec[0]), double(m_defaultValue.vec[1]), double(m_defaultValue.vec[2]));
}

Vec3dCVar::Vec3dCVar(atVec3d &value, std::string_view name, std::string_view description, CVar::EFlags flags)
    : CVar(name, description, flags)
    , m_value(value)
    , m_defaultValue(value)
{}

std::string Vec3dCVar::toString() const
{
    return athena::utility::sprintf("%lf %lf %lf", m_value.vec[0], m_value.vec[1], m_value.vec[2]);
}

bool Vec3dCVar::_fromString(std::string_view v)
{
    double x, y, z;
    if (std::sscanf(v.data(), "%lf %lf %lf", &x, &y, &z) != 3)
        return false;
    m_value.vec[0] = x;
    m_value.vec[1] = y;
    m_value.vec[2] = y;
    return true;
}

bool Vec3dCVar::hasDefaultValue() const
{
    return !memcmp(&m_value.vec, &m_defaultValue.vec[0], sizeof(atVec3d));
}

void Vec3dCVar::deserialize(athena::io::YAMLDocReader& reader)
{
    m_value = reader.readVec3d(m_name.c_str());
}

void Vec3dCVar::serialize(athena::io::YAMLDocWriter& writer) const
{
    writer.writeVec3d(m_name.c_str(), m_value);
}

std::string Vec3dCVar::defaultValueString() const
{
    return athena::utility::sprintf("%g %g %g", m_defaultValue.vec[0], m_defaultValue.vec[1], m_defaultValue.vec[2]);
}

Vec4fCVar::Vec4fCVar(atVec4f &value, std::string_view name, std::string_view description, CVar::EFlags flags)
    : CVar(name, description, flags)
    , m_value(value)
    , m_defaultValue(value)
{}

std::string Vec4fCVar::toString() const
{
    return athena::utility::sprintf("%g %g %g %g", &m_value.vec[0], &m_value.vec[1], &m_value.vec[2], &m_value.vec[3]);
}

bool Vec4fCVar::_fromString(std::string_view v)
{
    float x, y, z, w;
    if (std::sscanf(v.data(), "%f %f %f %f", &x, &y, &z, &w) != 4)
        return false;
    m_value.vec[0] = x;
    m_value.vec[1] = y;
    m_value.vec[2] = y;
    m_value.vec[3] = w;
    return true;
}

bool Vec4fCVar::hasDefaultValue() const
{
    return !memcmp(&m_value.vec, &m_defaultValue.vec[0], sizeof(atVec4f));
}

void Vec4fCVar::deserialize(athena::io::YAMLDocReader& reader)
{
    m_value = reader.readVec4f(m_name.c_str());
}

void Vec4fCVar::serialize(athena::io::YAMLDocWriter& writer) const
{
    writer.writeVec4f(m_name.c_str(), m_value);
}

std::string Vec4fCVar::defaultValueString() const
{
    return athena::utility::sprintf("%f %f %f", &m_defaultValue.vec[0], &m_defaultValue.vec[1], &m_defaultValue.vec[2]);
}

Vec4dCVar::Vec4dCVar(atVec4d &value, std::string_view name, std::string_view description, CVar::EFlags flags)
: CVar(name, description, flags)
, m_value(value)
, m_defaultValue(value)
{}

std::string Vec4dCVar::toString() const
{
    return athena::utility::sprintf("%f %f %f %f", m_value.vec[0], m_value.vec[1], m_value.vec[2], m_value.vec[3]);
}

bool Vec4dCVar::_fromString(std::string_view v)
{
    double x, y, z, w;
    if (std::sscanf(v.data(), "%lf %lf %lf %lf", &x, &y, &z, &w) != 4)
        return false;
    m_value.vec[0] = x;
    m_value.vec[1] = y;
    m_value.vec[2] = y;
    m_value.vec[3] = w;
    return true;
}

bool Vec4dCVar::hasDefaultValue() const
{
    return !memcmp(&m_value.vec, &m_defaultValue.vec[0], sizeof(atVec4d));
}

void Vec4dCVar::deserialize(athena::io::YAMLDocReader& reader)
{
    m_value = reader.readVec4d(m_name.c_str());
}

void Vec4dCVar::serialize(athena::io::YAMLDocWriter& writer) const
{
    writer.writeVec4d(m_name.c_str(), m_value);
}

std::string Vec4dCVar::defaultValueString() const
{
    return athena::utility::sprintf("%g %g %g", m_defaultValue.vec[0], m_defaultValue.vec[1], m_defaultValue.vec[2]);
}

StringCVar::StringCVar(std::string& value, std::string_view name, std::string_view help, CVar::EFlags flags)
: CVar(name, help, flags)
, m_value(value)
, m_defaultValue(value)
{
}

std::string StringCVar::toString() const
{
    return m_value;
}

bool StringCVar::_fromString(std::string_view v)
{
    m_value = v;
    return true;
}

bool StringCVar::hasDefaultValue() const
{
    return m_value == m_defaultValue;
}

void StringCVar::deserialize(athena::io::YAMLDocReader& reader)
{
    m_value = reader.readString(m_name.c_str());
}

void StringCVar::serialize(athena::io::YAMLDocWriter& writer) const
{
    writer.writeString(m_name.c_str(), m_value.c_str());
}


template class TCVar<bool>;
template class TCVar<int16_t>;
template class TCVar<uint16_t>;
template class TCVar<int32_t>;
template class TCVar<uint32_t>;
template class TCVar<int64_t>;
template class TCVar<uint64_t>;
template class TCVar<float>;
template class TCVar<double>;

}

