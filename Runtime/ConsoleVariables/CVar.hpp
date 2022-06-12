#pragma once

#include "Runtime/GCNTypes.hpp"
#include "zeus/zeus.hpp"

#include <functional>
#include <string>
#include <vector>
namespace metaforce {
namespace StoreCVar {
enum class EType : uint32_t { Boolean, Signed, Unsigned, Real, Literal, Vec2f, Vec2d, Vec3f, Vec3d, Vec4f, Vec4d };
enum class EFlags {
  None = 0,
  System = (1 << 0),
  Game = (1 << 1),
  Editor = (1 << 2),
  Gui = (1 << 3),
  Cheat = (1 << 4),
  Hidden = (1 << 5),
  ReadOnly = (1 << 6),
  Archive = (1 << 7),
  InternalArchivable = (1 << 8),
  Modified = (1 << 9),
  ModifyRestart = (1 << 10), //!< If this bit is set, any modification will inform the user that a restart is required
  Color = (1 << 11), //!< If this bit is set, Vec3f and Vec4f will be displayed in the console with a colored square
  NoDeveloper = (1 << 12), //!< Not even developer mode can modify this
  Any = -1
};
ENABLE_BITWISE_ENUM(EFlags)

class CVar {
public:
  std::string m_name;
  std::string m_value;
};

struct CVarContainer {
  u32 magic = 'CVAR';
  std::vector<CVar> cvars;
};

} // namespace StoreCVar

class CVarManager;
class ICVarValueReference;
class CVar : protected StoreCVar::CVar {
  friend class CVarManager;

public:
  typedef std::function<void(CVar*)> ListenerFunc;

  using EType = StoreCVar::EType;
  using EFlags = StoreCVar::EFlags;

  CVar(std::string_view name, std::string_view value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const zeus::CVector2f& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const zeus::CVector2d& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const zeus::CVector3f& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const zeus::CVector3d& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const zeus::CVector4f& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const zeus::CVector4d& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, double value, std::string_view help, EFlags flags);
  CVar(std::string_view name, bool value, std::string_view help, EFlags flags);
  CVar(std::string_view name, int32_t value, std::string_view help, EFlags flags);
  CVar(std::string_view name, uint32_t value, std::string_view help, EFlags flags);

  std::string_view name() const { return m_name; }
  std::string_view rawHelp() const { return m_help; }
  std::string_view defaultValue() const { return m_defaultValue; }
  std::string help() const;
  std::string value() const { return m_value; }

  template <typename T>
  inline bool toValue(T& value) const;
  zeus::CVector2f toVec2f(bool* isValid = nullptr) const;
  zeus::CVector2d toVec2d(bool* isValid = nullptr) const;
  zeus::CVector3f toVec3f(bool* isValid = nullptr) const;
  zeus::CVector3d toVec3d(bool* isValid = nullptr) const;
  zeus::CVector4f toVec4f(bool* isValid = nullptr) const;
  zeus::CVector4d toVec4d(bool* isValid = nullptr) const;
  double toReal(bool* isValid = nullptr) const;
  bool toBoolean(bool* isValid = nullptr) const;
  int32_t toSigned(bool* isValid = nullptr) const;
  uint32_t toUnsigned(bool* isValid = nullptr) const;
  std::string toLiteral(bool* isValid = nullptr) const;

  template <typename T>
  inline bool fromValue(T value) {
    return false;
  }
  bool fromVec2f(const zeus::CVector2f& val);
  bool fromVec2d(const zeus::CVector2d& val);
  bool fromVec3f(const zeus::CVector3f& val);
  bool fromVec3d(const zeus::CVector3d& val);
  bool fromVec4f(const zeus::CVector4f& val);
  bool fromVec4d(const zeus::CVector4d& val);
  bool fromReal(double val);
  bool fromBoolean(bool val);
  bool fromInteger(int32_t val);
  bool fromInteger(uint32_t val);
  bool fromLiteral(std::string_view val);
  bool fromLiteralToType(std::string_view val);

  bool isVec2f() const { return m_type == EType::Vec2f; }
  bool isVec2d() const { return m_type == EType::Vec2d; }
  bool isVec3f() const { return m_type == EType::Vec3f; }
  bool isVec3d() const { return m_type == EType::Vec3d; }
  bool isVec4f() const { return m_type == EType::Vec4f; }
  bool isVec4d() const { return m_type == EType::Vec4d; }
  bool isFloat() const { return m_type == EType::Real; }
  bool isBoolean() const { return m_type == EType::Boolean; }
  bool isInteger() const { return m_type == EType::Signed || m_type == EType::Unsigned; }
  bool isLiteral() const { return m_type == EType::Literal; }
  bool isModified() const;
  bool modificationRequiresRestart() const;
  bool isReadOnly() const;
  bool isCheat() const;
  bool isHidden() const;
  bool isArchive() const;
  bool isInternalArchivable() const;
  bool isNoDeveloper() const;
  bool isColor() const;
  bool wasDeserialized() const;
  bool hasDefaultValue() const;

  EType type() const { return m_type; }
  EFlags flags() const { return (m_unlocked ? m_oldFlags : m_flags); }

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

  void addListener(ListenerFunc func) { m_listeners.push_back(std::move(func)); }
  void addVariableReference(ICVarValueReference* v) { m_valueReferences.push_back(v); }
  void removeVariableReference(ICVarValueReference* v) {
    auto it = std::find(m_valueReferences.begin(), m_valueReferences.end(), v);
    if (it != m_valueReferences.end()) {
      m_valueReferences.erase(it);
    }
  }

  bool isValidInput(std::string_view input) const;

private:
  CVar(std::string_view name, std::string_view help, EType type) : m_help(help), m_type(type) { m_name = name; }
  void dispatch();
  void clearModified();
  void forceClearModified();
  void setModified();
  std::string m_help;
  EType m_type;
  std::string m_defaultValue;
  EFlags m_flags = EFlags::None;
  EFlags m_oldFlags = EFlags::None;
  bool m_unlocked = false;
  bool m_wasDeserialized = false;
  std::vector<ListenerFunc> m_listeners;
  std::vector<ICVarValueReference*> m_valueReferences;
  bool safeToModify(EType type) const;
  void init(EFlags flags, bool removeColor = true);
};

template <>
inline bool CVar::toValue(zeus::CVector2f& value) const {
  bool isValid = false;
  value = toVec2f(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(zeus::CVector2d& value) const {
  bool isValid = false;
  value = toVec2d(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(zeus::CVector3f& value) const {
  bool isValid = false;
  value = toVec3f(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(zeus::CVector3d& value) const {
  bool isValid = false;
  value = toVec3d(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(zeus::CVector4f& value) const {
  bool isValid = false;
  value = toVec4f(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(zeus::CVector4d& value) const {
  bool isValid = false;
  value = toVec4d(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(double& value) const {
  bool isValid = false;
  value = toReal(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(float& value) const {
  bool isValid = false;
  value = static_cast<float>(toReal(&isValid));
  return isValid;
}
template <>
inline bool CVar::toValue(bool& value) const {
  bool isValid = false;
  value = toBoolean(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(int32_t& value) const {
  bool isValid = false;
  value = toSigned(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(uint32_t& value) const {
  bool isValid = false;
  value = toUnsigned(&isValid);
  return isValid;
}
template <>
inline bool CVar::toValue(std::string& value) const {
  bool isValid = false;
  value = toLiteral(&isValid);
  return isValid;
}

template <>
inline bool CVar::fromValue(const zeus::CVector2f& val) {
  return fromVec2f(val);
}
template <>
inline bool CVar::fromValue(const zeus::CVector2d& val) {
  return fromVec2d(val);
}
template <>
inline bool CVar::fromValue(const zeus::CVector3f& val) {
  return fromVec3f(val);
}
template <>
inline bool CVar::fromValue(const zeus::CVector3d& val) {
  return fromVec3d(val);
}
template <>
inline bool CVar::fromValue(const zeus::CVector4f& val) {
  return fromVec4f(val);
}
template <>
inline bool CVar::fromValue(const zeus::CVector4d& val) {
  return fromVec4d(val);
}
template <>
inline bool CVar::fromValue(float val) {
  return fromReal(val);
}
template <>
inline bool CVar::fromValue(double val) {
  return fromReal(val);
}
template <>
inline bool CVar::fromValue(bool val) {
  return fromBoolean(val);
}
template <>
inline bool CVar::fromValue(int32_t val) {
  return fromInteger(val);
}
template <>
inline bool CVar::fromValue(uint32_t val) {
  return fromInteger(val);
}
template <>
inline bool CVar::fromValue(std::string_view val) {
  return fromLiteral(val);
}

class CVarUnlocker {
  CVar* m_cvar;

public:
  CVarUnlocker(CVar* cvar) : m_cvar(cvar) {
    if (m_cvar)
      m_cvar->unlock();
  }
  ~CVarUnlocker() {
    if (m_cvar)
      m_cvar->lock();
  }
};
class ICVarValueReference {
protected:
  CVar* m_cvar = nullptr;

public:
  ICVarValueReference() = default;
  explicit ICVarValueReference(CVar* cv) : m_cvar(cv) {
    if (m_cvar != nullptr) {
      m_cvar->addVariableReference(this);
    }
  }
  virtual ~ICVarValueReference() {
    if (m_cvar != nullptr) {
      m_cvar->removeVariableReference(this);
    }
    m_cvar = nullptr;
  }
  virtual void updateValue() = 0;
};

template <typename T>
class CVarValueReference : public ICVarValueReference {
  T* m_valueRef = nullptr;

public:
  CVarValueReference() = default;
  explicit CVarValueReference(T* t, CVar* cv) : ICVarValueReference(cv) {
    m_valueRef = t;
    if (m_valueRef && m_cvar) {
      m_cvar->toValue(*m_valueRef);
    }
  }

  void updateValue() override {
    if (m_valueRef != nullptr && m_cvar->isModified()) {
      m_cvar->toValue(*m_valueRef);
    }
  }
};
} // namespace metaforce
