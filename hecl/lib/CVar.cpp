#include "hecl/CVar.hpp"

#include <sstream>

#include "hecl/CVarManager.hpp"
#include "hecl/hecl.hpp"

#include <athena/Utility.hpp>

namespace hecl {
extern CVar* com_developer;
extern CVar* com_enableCheats;

using namespace std::literals;

CVar::CVar(std::string_view name, std::string_view value, std::string_view help, CVar::EFlags flags)
: CVar(name, help, EType::Literal) {
  fromLiteral(value);
  init(flags);
}

CVar::CVar(std::string_view name, const atVec2f& value, std::string_view help, EFlags flags)
: CVar(name, help, EType::Vec2f) {
  fromVec2f(value);
  init(flags);
}

CVar::CVar(std::string_view name, const atVec2d& value, std::string_view help, EFlags flags)
: CVar(name, help, EType::Vec2d) {
  fromVec2d(value);

  init(flags);
}

CVar::CVar(std::string_view name, const atVec3f& value, std::string_view help, EFlags flags)
: CVar(name, help, EType::Vec3f) {
  fromVec3f(value);
  init(flags, false);
}

CVar::CVar(std::string_view name, const atVec3d& value, std::string_view help, EFlags flags)
: CVar(name, help, EType::Vec3d) {
  fromVec3d(value);
  init(flags, false);
}

CVar::CVar(std::string_view name, const atVec4f& value, std::string_view help, EFlags flags)
: CVar(name, help, EType::Vec4f) {
  fromVec4f(value);
  init(flags, false);
}

CVar::CVar(std::string_view name, const atVec4d& value, std::string_view help, EFlags flags)
: CVar(name, help, EType::Vec4d) {
  fromVec4d(value);
  init(flags, false);
}

CVar::CVar(std::string_view name, double value, std::string_view help, EFlags flags) : CVar(name, help, EType::Real) {
  fromReal(value);
  init(flags);
}

CVar::CVar(std::string_view name, bool value, std::string_view help, CVar::EFlags flags)
: CVar(name, help, EType::Boolean) {
  fromBoolean(value);
  init(flags);
}

CVar::CVar(std::string_view name, int32_t value, std::string_view help, CVar::EFlags flags)
: CVar(name, help, EType::Signed) {
  fromInteger(value);
  init(flags);
}

CVar::CVar(std::string_view name, uint32_t value, std::string_view help, CVar::EFlags flags)
: CVar(name, help, EType::Unsigned) {
  fromInteger(value);
  init(flags);
}

std::string CVar::help() const {
  return m_help + (m_defaultValue.empty() ? "" : "\ndefault: " + m_defaultValue) + (isReadOnly() ? " [ReadOnly]" : "");
}

atVec2f CVar::toVec2f(bool* isValid) const {
  if (m_type != EType::Vec2f) {
    if (isValid != nullptr)
      *isValid = false;

    return atVec2f{};
  }

  if (isValid != nullptr)
    *isValid = true;

  atVec2f vec{};
  athena::simd_floats f;
  std::sscanf(m_value.c_str(), "%g %g", &f[0], &f[1]);
  vec.simd.copy_from(f);

  return vec;
}

atVec2d CVar::toVec2d(bool* isValid) const {
  if (m_type != EType::Vec2d) {
    if (isValid != nullptr)
      *isValid = false;

    return atVec2d{};
  }

  if (isValid != nullptr)
    *isValid = true;

  atVec2d vec{};
  athena::simd_doubles f;
  std::sscanf(m_value.c_str(), "%lg %lg", &f[0], &f[1]);
  vec.simd.copy_from(f);

  return vec;
}

atVec3f CVar::toVec3f(bool* isValid) const {
  if (m_type != EType::Vec3f) {
    if (isValid != nullptr)
      *isValid = false;

    return atVec3f{};
  }

  if (isValid != nullptr)
    *isValid = true;

  atVec3f vec{};
  athena::simd_floats f;
  std::sscanf(m_value.c_str(), "%g %g %g", &f[0], &f[1], &f[2]);
  vec.simd.copy_from(f);

  return vec;
}

atVec3d CVar::toVec3d(bool* isValid) const {
  if (m_type != EType::Vec3d) {
    if (isValid != nullptr)
      *isValid = false;

    return atVec3d{};
  }

  if (isValid != nullptr)
    *isValid = true;

  atVec3d vec{};
  athena::simd_doubles f;
  std::sscanf(m_value.c_str(), "%lg %lg %lg", &f[0], &f[1], &f[2]);
  vec.simd.copy_from(f);

  return vec;
}

atVec4f CVar::toVec4f(bool* isValid) const {
  if (m_type != EType::Vec4f) {
    if (isValid != nullptr)
      *isValid = false;

    return atVec4f{};
  }

  if (isValid != nullptr)
    *isValid = true;

  atVec4f vec{};
  athena::simd_floats f;
  std::sscanf(m_value.c_str(), "%g %g %g %g", &f[0], &f[1], &f[2], &f[3]);
  vec.simd.copy_from(f);

  return vec;
}

atVec4d CVar::toVec4d(bool* isValid) const {
  if (m_type != EType::Vec4d) {
    if (isValid != nullptr)
      *isValid = false;

    return atVec4d{};
  }

  if (isValid != nullptr)
    *isValid = true;

  atVec4d vec{};
  athena::simd_doubles f;
  std::sscanf(m_value.c_str(), "%lg %lg %lg %lg", &f[0], &f[1], &f[2], &f[3]);
  vec.simd.copy_from(f);

  return vec;
}

double CVar::toReal(bool* isValid) const {
  if (m_type != EType::Real) {
    if (isValid)
      *isValid = false;
    return 0.0f;
  }

  if (isValid != nullptr)
    *isValid = true;

  return strtod(m_value.c_str(), nullptr);
}

bool CVar::toBoolean(bool* isValid) const {
  if (m_type != EType::Boolean) {
    if (isValid)
      *isValid = false;

    return false;
  }

  if (isValid != nullptr)
    *isValid = true;

  return athena::utility::parseBool(m_value);
}

int32_t CVar::toSigned(bool* isValid) const {
  if (m_type != EType::Signed && m_type != EType::Unsigned) {
    if (isValid)
      *isValid = false;
    return 0;
  }

  if (isValid != nullptr)
    *isValid = true;

  return strtol(m_value.c_str(), nullptr, 0);
}

uint32_t CVar::toUnsigned(bool* isValid) const {
  if (m_type != EType::Signed && m_type != EType::Unsigned) {
    if (isValid)
      *isValid = false;
    return 0;
  }

  if (isValid != nullptr)
    *isValid = true;

  return strtoul(m_value.c_str(), nullptr, 0);
}

std::string CVar::toLiteral(bool* isValid) const {
  if (m_type != EType::Literal && (com_developer && com_developer->toBoolean())) {
    if (isValid != nullptr)
      *isValid = false;
  } else if (isValid != nullptr) {
    *isValid = true;
  }

  // Even if it's not a literal, it's still safe to return
  return m_value;
}

std::wstring CVar::toWideLiteral(bool* isValid) const {
  if (m_type != EType::Literal && (com_developer && com_developer->toBoolean())) {
    if (isValid != nullptr)
      *isValid = false;
  } else if (isValid != nullptr) {
    *isValid = true;
  }

  // Even if it's not a literal, it's still safe to return
  return hecl::UTF8ToWide(m_value);
}

bool CVar::fromVec2f(const atVec2f& val) {
  if (!safeToModify(EType::Vec2f))
    return false;

  athena::simd_floats f(val.simd);
  m_value.assign(fmt::format(fmt("{} {}"), f[0], f[1]));
  m_flags |= EFlags::Modified;
  return true;
}

bool CVar::fromVec2d(const atVec2d& val) {
  if (!safeToModify(EType::Vec2d))
    return false;

  athena::simd_doubles f(val.simd);
  m_value.assign(fmt::format(fmt("{} {}"), f[0], f[1]));
  m_flags |= EFlags::Modified;
  return true;
}

bool CVar::fromVec3f(const atVec3f& val) {
  if (!safeToModify(EType::Vec3f))
    return false;

  athena::simd_floats f(val.simd);
  m_value.assign(fmt::format(fmt("{} {} {}"), f[0], f[1], f[2]));
  m_flags |= EFlags::Modified;
  return true;
}

bool CVar::fromVec3d(const atVec3d& val) {
  if (!safeToModify(EType::Vec3d))
    return false;

  athena::simd_doubles f(val.simd);
  m_value.assign(fmt::format(fmt("{} {} {}"), f[0], f[1], f[2]));
  m_flags |= EFlags::Modified;
  return true;
}

bool CVar::fromVec4f(const atVec4f& val) {
  if (!safeToModify(EType::Vec4f))
    return false;

  athena::simd_floats f(val.simd);
  m_value.assign(fmt::format(fmt("{} {} {} {}"), f[0], f[1], f[2], f[3]));
  m_flags |= EFlags::Modified;
  return true;
}

bool CVar::fromVec4d(const atVec4d& val) {
  if (!safeToModify(EType::Vec4d))
    return false;

  athena::simd_doubles f(val.simd);
  m_value.assign(fmt::format(fmt("{} {} {} {}"), f[0], f[1], f[2], f[3]));
  m_flags |= EFlags::Modified;
  return true;
}

bool CVar::fromReal(double val) {
  if (!safeToModify(EType::Real))
    return false;

  m_value.assign(fmt::format(fmt("{}"), val));
  setModified();
  return true;
}

bool CVar::fromBoolean(bool val) {
  if (!safeToModify(EType::Boolean))
    return false;

  if (val)
    m_value = "true"sv;
  else
    m_value = "false"sv;

  setModified();
  return true;
}

bool CVar::fromInteger(int32_t val) {
  if ((com_developer && com_enableCheats) && (!com_developer->toBoolean() || !com_enableCheats->toBoolean()) &&
      isCheat())
    return false;

  // We'll accept both signed an unsigned input
  if (m_type != EType::Signed && m_type != EType::Unsigned)
    return false;

  if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
    return false;

  // Properly format based on signedness
  m_value = fmt::format(fmt("{}"), (m_type == EType::Signed ? val : static_cast<uint32_t>(val)));
  setModified();
  return true;
}

bool CVar::fromInteger(uint32_t val) {
  if ((com_developer && com_enableCheats) && (!com_developer->toBoolean() || !com_enableCheats->toBoolean()) &&
      isCheat())
    return false;

  // We'll accept both signed an unsigned input
  if (m_type != EType::Signed && m_type != EType::Unsigned)
    return false;

  if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
    return false;

  // Properly format based on signedness
  m_value = fmt::format(fmt("{}"), (m_type == EType::Unsigned ? val : static_cast<int32_t>(val)));
  setModified();
  return true;
}

bool CVar::fromLiteral(std::string_view val) {
  if (!safeToModify(EType::Literal))
    return false;

  m_value.assign(val);
  setModified();
  return true;
}

bool CVar::fromLiteral(std::wstring_view val) {
  if (!safeToModify(EType::Literal))
    return false;

  m_value.assign(hecl::WideToUTF8(val));
  setModified();
  return true;
}

bool CVar::fromLiteralToType(std::string_view val) {
  if (!safeToModify(m_type) || !isValidInput(val))
    return false;
  m_value = val;
  setModified();
  return true;
}

bool CVar::fromLiteralToType(std::wstring_view val) {
  return fromLiteralToType(hecl::WideToUTF8(val));
}

bool CVar::isModified() const { return True(m_flags & EFlags::Modified); }
bool CVar::modificationRequiresRestart() const { return True(m_flags & EFlags::ModifyRestart); }

bool CVar::isReadOnly() const { return True(m_flags & EFlags::ReadOnly); }

bool CVar::isCheat() const { return True(m_flags & EFlags::Cheat); }

bool CVar::isHidden() const { return True(m_flags & EFlags::Hidden); }

bool CVar::isArchive() const { return True(m_flags & EFlags::Archive); }

bool CVar::isInternalArchivable() const { return True(m_flags & EFlags::InternalArchivable); }

bool CVar::isColor() const {
  return True(m_flags & EFlags::Color) &&
         (m_type == EType::Vec3f || m_type == EType::Vec3d || m_type == EType::Vec3f || m_type == EType::Vec4d);
}

bool CVar::isNoDeveloper() const { return True(m_flags & EFlags::NoDeveloper); }

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


bool isReal(std::string_view v) {
  char* p;
  std::strtod(v.data(), &p);
  return *p == 0;
}
bool isReal(const std::vector<std::string>& v) {
  for (auto& s : v) {
    if (!isReal(s))
      return false;
  }
  return true;
}

bool CVar::isValidInput(std::string_view input) const {
  std::vector<std::string> parts = athena::utility::split(input, ' ');
  char* p;
  switch(m_type) {
  case EType::Boolean: {
    bool valid = false;
    athena::utility::parseBool(input, &valid);
    return valid;
  }
  case EType::Signed:
    std::strtol(input.data(), &p, 0);
    return p == nullptr;
  case EType::Unsigned:
    std::strtoul(input.data(), &p, 0);
    return p == nullptr;
  case EType::Real: {
    bool size = parts.size() == 1;
    bool ret = isReal(input);
    return ret && size;
  }
  case EType::Literal:
    return true;
  case EType::Vec2f:
  case EType::Vec2d:
    return parts.size() == 2 && isReal(parts);
  case EType::Vec3f:
  case EType::Vec3d:
    return parts.size() == 3 && isReal(parts);
  case EType::Vec4f:
  case EType::Vec4d:
    return parts.size() == 4 && isReal(parts);
  }

  return false;
}

bool CVar::isValidInput(std::wstring_view input) const {
  return isValidInput(hecl::WideToUTF8(input));
}

bool CVar::safeToModify(EType type) const {
  // Are we NoDevelper?
  if (isNoDeveloper())
    return false;

  // Are we a cheat?
  if (isCheat() && (com_developer && com_enableCheats) &&
      (!com_developer->toBoolean() || !com_enableCheats->toBoolean()))
    return false;

  // Are we read only?
  if (isReadOnly() && (com_developer && !com_developer->toBoolean()))
    return false;

  return m_type == type;
}

void CVar::init(EFlags flags, bool removeColor) {
  m_defaultValue = m_value;
  m_flags = flags;
  if (removeColor) {
    // If the user specifies color, we don't want it
    m_flags &= ~EFlags::Color;
  }
}

} // namespace hecl
