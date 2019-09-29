#pragma once

#include <functional>
#include <string>
#include <vector>

#include <athena/DNAYaml.hpp>
#include <athena/Global.hpp>
#include <athena/Types.hpp>

namespace hecl {
namespace DNACVAR {
enum class EType : atUint8 { Boolean, Signed, Unsigned, Real, Literal, Vec2f, Vec2d, Vec3f, Vec3d, Vec4f, Vec4d };

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

class CVar : public athena::io::DNA<athena::Endian::Big> {
public:
  AT_DECL_DNA
  String<-1> m_name;
  String<-1> m_value;
};

struct CVarContainer : public athena::io::DNA<athena::Endian::Big> {
  AT_DECL_DNA
  Value<atUint32> magic = 'CVAR';
  Value<atUint32> cvarCount;
  Vector<CVar, AT_DNA_COUNT(cvarCount)> cvars;
};

} // namespace DNACVAR

class CVarManager;
class CVar : protected DNACVAR::CVar {
  friend class CVarManager;
  Delete _d;

public:
  typedef std::function<void(CVar*)> ListenerFunc;

  using EType = DNACVAR::EType;
  using EFlags = DNACVAR::EFlags;

  CVar(std::string_view name, std::string_view value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const atVec2f& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const atVec2d& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const atVec3f& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const atVec3d& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const atVec4f& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, const atVec4d& value, std::string_view help, EFlags flags);
  CVar(std::string_view name, double value, std::string_view help, EFlags flags);
  CVar(std::string_view name, bool value, std::string_view help, EFlags flags);
  CVar(std::string_view name, int32_t value, std::string_view help, EFlags flags);
  CVar(std::string_view name, uint32_t value, std::string_view help, EFlags flags);

  std::string_view name() const { return m_name; }
  std::string_view rawHelp() const { return m_help; }
  std::string help() const;
  std::string value() const { return m_value; }

  atVec2f toVec2f(bool* isValid = nullptr) const;
  atVec2d toVec2d(bool* isValie = nullptr) const;
  atVec3f toVec3f(bool* isValid = nullptr) const;
  atVec3d toVec3d(bool* isValie = nullptr) const;
  atVec4f toVec4f(bool* isValid = nullptr) const;
  atVec4d toVec4d(bool* isValie = nullptr) const;
  double toReal(bool* isValid = nullptr) const;
  bool toBoolean(bool* isValid = nullptr) const;
  int32_t toSigned(bool* isValid = nullptr) const;
  uint32_t toUnsigned(bool* isValid = nullptr) const;
  std::wstring toWideLiteral(bool* isValid = nullptr) const;
  std::string toLiteral(bool* isValid = nullptr) const;

  bool fromVec2f(const atVec2f& val);
  bool fromVec2d(const atVec2d& val);
  bool fromVec3f(const atVec3f& val);
  bool fromVec3d(const atVec3d& val);
  bool fromVec4f(const atVec4f& val);
  bool fromVec4d(const atVec4d& val);
  bool fromReal(double val);
  bool fromBoolean(bool val);
  bool fromInteger(int32_t val);
  bool fromInteger(uint32_t val);
  bool fromLiteral(std::string_view val);
  bool fromLiteral(std::wstring_view val);
  bool fromLiteralToType(std::string_view val);
  bool fromLiteralToType(std::wstring_view val);

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
  void clearModified();
  void setModified();

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

  bool isValidInput(std::string_view input) const;
  bool isValidInput(std::wstring_view input) const;

private:
  CVar(std::string_view name, std::string_view help, EType type) : m_help(help), m_type(type) { m_name = name; }
  void dispatch();
  std::string m_help;
  EType m_type;
  std::string m_defaultValue;
  EFlags m_flags = EFlags::None;
  EFlags m_oldFlags = EFlags::None;
  bool m_unlocked = false;
  bool m_wasDeserialized = false;
  std::vector<ListenerFunc> m_listeners;
  bool safeToModify(EType type) const;
  void init(EFlags flags, bool removeColor=true);
};

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

} // namespace hecl
