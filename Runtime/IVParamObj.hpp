#pragma once

#include <memory>
#include "IObj.hpp"

namespace urde {

class IVParamObj : public IObj {
public:
  virtual ~IVParamObj() {}
};

template <class T>
class TObjOwnerParam : public IVParamObj {
  T m_param;

public:
  TObjOwnerParam(T&& obj) : m_param(std::move(obj)) {}
  T& GetParam() { return m_param; }
};

class CVParamTransfer {
  std::shared_ptr<IVParamObj> m_ref;

public:
  CVParamTransfer() = default;
  CVParamTransfer(IVParamObj* obj) : m_ref(obj) {}
  CVParamTransfer(const CVParamTransfer& other) : m_ref(other.m_ref) {}
  IVParamObj* GetObj() const { return m_ref.get(); }
  CVParamTransfer ShareTransferRef() { return CVParamTransfer(*this); }

  template <class T>
  T& GetOwnedObj() const {
    return static_cast<TObjOwnerParam<T>*>(GetObj())->GetParam();
  }

  static CVParamTransfer Null() { return CVParamTransfer(); }
};

} // namespace urde
