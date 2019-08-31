#pragma once

#include "RetroTypes.hpp"

namespace urde {

class IObj {
public:
  virtual ~IObj() = default;
};

class TObjOwnerDerivedFromIObjUntyped : public IObj {
protected:
  void* m_objPtr;

public:
  TObjOwnerDerivedFromIObjUntyped(void* objPtr) : m_objPtr(objPtr) {}
};

template <class T>
class TObjOwnerDerivedFromIObj : public TObjOwnerDerivedFromIObjUntyped {
  TObjOwnerDerivedFromIObj(T* objPtr) : TObjOwnerDerivedFromIObjUntyped(objPtr) {}

public:
  static std::unique_ptr<TObjOwnerDerivedFromIObj<T>> GetNewDerivedObject(std::unique_ptr<T>&& obj) {
    return std::unique_ptr<TObjOwnerDerivedFromIObj<T>>(new TObjOwnerDerivedFromIObj<T>(obj.release()));
  }
  ~TObjOwnerDerivedFromIObj() override { std::default_delete<T>()(static_cast<T*>(m_objPtr)); }
  T* GetObj() { return static_cast<T*>(m_objPtr); }
};

} // namespace urde
