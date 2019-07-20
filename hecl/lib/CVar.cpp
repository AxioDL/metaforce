#include "hecl/hecl.hpp"
#include "hecl/CVar.hpp"
#include "hecl/CVarManager.hpp"

#include <athena/Utility.hpp>
#include <algorithm>
#include <sstream>

namespace hecl {
extern CVar* com_developer;
extern CVar* com_enableCheats;

using namespace std::literals;

CVar::CVar(std::string_view name, std::string_view value, std::string_view help, EType type, EFlags flags,
           CVarManager& parent)
: m_mgr(parent) {
  m_name = std::string(name);
  m_value = std::string(value);
  m_defaultValue = std::string(value);
  m_help = help;
  m_type = type;
  m_flags = flags;
}

CVar::CVar(std::string_view name, std::string_view value, std::string_view help, CVar::EFlags flags,
           CVarManager& parent)
: m_mgr(parent) {
  m_flags = flags;
  m_name = std::string(name);
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

CVar::CVar(std::string_view name, const atVec4f& value, std::string_view help, EFlags flags, CVarManager& parent)
: m_mgr(parent) {
  m_name = std::string(name);
  m_help = help;
  m_type = EType::Vec4f;
  m_flags = flags;

  // Unlock the cvar for writing if readonly
  unlock();

  fromVec4f(value);
  m_defaultValue = m_value;

  // Lock the cvar
  lock();
  // Clear the modified flag, just incase lock didn't do it.
  m_flags = flags;
}

CVar::CVar(std::string_view name, float value, std::string_view help, EFlags flags, CVarManager& parent)
: m_mgr(parent) {
  m_name = std::string(name);
  m_help = help;
  m_type = EType::Float;
  m_flags = flags;

  // Unlock the cvar for writing if readonly
  unlock();

  fromFloat(value);
  m_defaultValue = m_value;

  // Lock the cvar
  lock();
  // Clear the modified flag, just incase lock didn't do it.
  m_flags = flags;
}

CVar::CVar(std::string_view name, bool value, std::string_view help, CVar::EFlags flags, CVarManager& parent)
: m_mgr(parent) {
  m_name = std::string(name);
  m_help = help;
  m_type = EType::Boolean;
  m_flags = flags;

  // Unlock the cvar for writing if readonly
  unlock();

  fromBoolean(value);
  m_defaultValue = m_value;

  // Lock the cvar
  lock();
  // Clear the modified flag, just incase lock didn't do it.
  m_flags = flags;
}

CVar::CVar(std::string_view name, int value, std::string_view help, CVar::EFlags flags, CVarManager& parent)
: m_mgr(parent) {
  m_name = std::string(name);
  m_help = help;
  m_type = EType::Integer;
  m_flags = flags;

  // Unlock the cvar for writing if readonly
  unlock();

  fromInteger(value);
  m_defaultValue = m_value;

  // Lock the cvar
  lock();
  // Clear the modified flag, just incase lock didn't do it.
  m_flags = flags;
}

std::string CVar::help() const {
  return std::string(m_help + (m_defaultValue != std::string() ? "\ndefault: " + m_defaultValue : "") +
                     (isReadOnly() ? " [ReadOnly]" : ""));
}

atVec4f CVar::toVec4f(bool* isValid) const {
  if (m_type != EType::Vec4f) {
    if (isValid != nullptr)
      *isValid = false;

    return atVec4f{};
  }

  if (isValid != nullptr)
    *isValid = true;

  atVec4f vec;
  athena::simd_floats f;
  std::sscanf(m_value.c_str(), "%f %f %f %f", &f[0], &f[1], &f[2], &f[3]);
  vec.simd.copy_from(f);

  return vec;
}

float CVar::toFloat(bool* isValid) const {
  if (m_type != EType::Float) {
    if (isValid)
      *isValid = false;
    return 0.0f;
  }

  return strtof(m_value.c_str(), nullptr);
}

bool CVar::toBoolean(bool* isValid) const {
  if (m_type != EType::Boolean) {
    if (isValid)
      *isValid = false;

    return false;
  }

  // We don't want to modify the original value;
  std::string tmp = m_value;
  athena::utility::tolower(tmp);

  if (!tmp.compare("yes") || !tmp.compare("true") || !tmp.compare("1")) {
    if (isValid)
      *isValid = true;
    return true;
  } else if (!tmp.compare("no") || !tmp.compare("false") || !tmp.compare("0")) {
    if (isValid)
      *isValid = true;
    return false;
  }

  if (isValid)
    *isValid = false;

  return false;
}

int CVar::toInteger(bool* isValid) const {
  if (m_type != EType::Integer) {
    if (isValid)
      *isValid = false;
    return 0;
  }

  return strtol(m_value.c_str(), nullptr, 0);
}

const std::string CVar::toLiteral(bool* isValid) const {
  if (m_type != EType::Literal && (com_developer && com_developer->toBoolean())) {
    if (isValid != nullptr)
      *isValid = false;
  } else if (isValid != nullptr)
    *isValid = true;

  // Even if it's not a literal, it's still safe to return
  return m_value;
}

const std::wstring CVar::toWideLiteral(bool* isValid) const {
  if (m_type != EType::Literal && (com_developer && com_developer->toBoolean())) {
    if (isValid != nullptr)
      *isValid = false;
  } else if (isValid != nullptr)
    *isValid = true;

  // Even if it's not a literal, it's still safe to return
  return hecl::UTF8ToWide(m_value);
}

bool CVar::fromVec4f(const atVec4f& val) {
  if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
    return false;
  else if (isCheat())
    return false;

  if (m_type != EType::Vec4f)
    return false;

  if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
    return false;

  athena::simd_floats f(val.simd);
  m_value.assign(fmt::format(fmt("{} {} {} {}"), f[0], f[1], f[2], f[3]));
  m_flags |= EFlags::Modified;
  return true;
}

bool CVar::fromFloat(float val) {
  if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
    return false;
  else if (isCheat())
    return false;

  if (m_type != EType::Float)
    return false;

  if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
    return false;

  m_value.assign(fmt::format(fmt("{}"), val));
  setModified();
  return true;
}

bool CVar::fromBoolean(bool val) {
  if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
    return false;
  else if (isCheat())
    return false;

  if (m_type != EType::Boolean)
    return false;

  if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
    return false;

  if (val)
    m_value = "true"sv;
  else
    m_value = "false"sv;

  setModified();
  return true;
}

bool CVar::fromInteger(int val) {
  if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
    return false;
  else if (isCheat())
    return false;

  if (m_type != EType::Integer)
    return false;

  if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
    return false;

  m_value = fmt::format(fmt("{}"), val);
  setModified();
  return true;
}

bool CVar::fromLiteral(std::string_view val) {
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

bool CVar::fromLiteral(std::wstring_view val) {
  if (isCheat() && (com_developer && !com_developer->toBoolean() && !com_enableCheats->toBoolean()))
    return false;
  else if (isCheat())
    return false;

  if (m_type != EType::Literal)
    return false;

  if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
    return false;

  m_value.assign(hecl::WideToUTF8(val));
  setModified();
  return true;
}

bool CVar::fromLiteralToType(std::string_view val, bool setDefault) {
  switch (m_type) {
  case EType::Literal:
    return fromLiteral(val);
  case EType::Boolean: {
    std::stringstream ss;
    ss << std::boolalpha << val;
    bool v;
    ss >> v;
    return fromBoolean(v);
  }
  case EType::Float: {
    std::stringstream ss;
    ss << val;
    float v;
    ss >> v;
    return fromFloat(v);
  }
  case EType::Integer: {
    std::stringstream ss;
    ss << val;
    int v;
    ss >> v;
    return fromInteger(v);
  }
  case EType::Vec4f: {
    atVec4f vec;
    athena::simd_floats f;
    std::sscanf(val.data(), "%f %f %f %f", &f[0], &f[1], &f[2], &f[3]);
    vec.simd.copy_from(f);
    return fromVec4f(vec);
  }
  }
  if (setDefault)
    m_value = m_defaultValue;
  return false;
}

bool CVar::fromLiteralToType(std::wstring_view val, bool setDefault) {
  switch (m_type) {
  case EType::Literal:
    return fromLiteral(val);
  case EType::Boolean: {
    std::wstringstream ss;
    ss << std::boolalpha << val;
    bool v;
    ss >> v;
    return fromBoolean(v);
  }
  case EType::Float: {
    std::wstringstream ss;
    ss << val;
    float v;
    ss >> v;
    return fromFloat(v);
  }
  case EType::Integer: {
    std::wstringstream ss;
    ss << val;
    int v;
    ss >> v;
    return fromInteger(v);
  }
  case EType::Vec4f: {
    atVec4f vec;
    athena::simd_floats f;
    std::swscanf(val.data(), L"%f %f %f %f", &f[0], &f[1], &f[2], &f[3]);
    vec.simd.copy_from(f);
    return fromVec4f(vec);
  }
  }
  if (setDefault)
    m_value = m_defaultValue;
  return false;
}

bool CVar::isModified() const { return int(m_flags & EFlags::Modified) != 0; }
bool CVar::modificationRequiresRestart() const { return int(m_flags & EFlags::ModifyRestart) != 0; }

bool CVar::isReadOnly() const { return int(m_flags & EFlags::ReadOnly) != 0; }

bool CVar::isCheat() const { return int(m_flags & EFlags::Cheat) != 0; }

bool CVar::isHidden() const { return int(m_flags & EFlags::Hidden) != 0; }

bool CVar::isArchive() const { return int(m_flags & EFlags::Archive) != 0; }

bool CVar::isInternalArchivable() const { return int(m_flags & EFlags::InternalArchivable) != 0; }

bool CVar::wasDeserialized() const { return m_wasDeserialized; }

bool CVar::hasDefaultValue() const { return m_defaultValue == m_value; }

void CVar::clearModified() {
  if (!modificationRequiresRestart())
    m_flags &= ~EFlags::Modified;
}

void CVar::setModified() { m_flags |= EFlags::Modified; }

void CVar::unlock() {
  if (isReadOnly() && !m_unlocked) {
    m_oldFlags = m_flags;
    m_flags &= ~EFlags::ReadOnly;
    m_unlocked = true;
  }
}

void CVar::lock() {
  if (!isReadOnly() && m_unlocked) {
    m_flags = m_oldFlags;
    m_unlocked = false;
  }
}

void CVar::dispatch() {
  for (const ListenerFunc& listen : m_listeners)
    listen(this);
}
} // namespace hecl
