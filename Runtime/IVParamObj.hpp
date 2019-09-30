#pragma once

#include <memory>
#include "Runtime/IObj.hpp"

namespace urde {

class IVParamObj : public IObj {
public:
  ~IVParamObj() override = default;
};

template <class T>
class TObjOwnerParam : public IVParamObj {
  T m_param;

public:
  TObjOwnerParam(T&& obj) : m_param(std::move(obj)) {}
  T& GetParam() noexcept { return m_param; }
  const T& GetParam() const noexcept { return m_param; }
};

class CVParamTransfer {
  std::shared_ptr<IVParamObj> m_ref;

public:
  constexpr CVParamTransfer() noexcept = default;
  CVParamTransfer(IVParamObj* obj) : m_ref(obj) {}

  CVParamTransfer(const CVParamTransfer& other) noexcept = default;
  CVParamTransfer& operator=(const CVParamTransfer&) noexcept = default;

  CVParamTransfer(CVParamTransfer&&) noexcept = default;
  CVParamTransfer& operator=(CVParamTransfer&&) noexcept = default;

  IVParamObj* GetObj() const noexcept { return m_ref.get(); }
  CVParamTransfer ShareTransferRef() const noexcept { return CVParamTransfer(*this); }

  template <class T>
  T& GetOwnedObj() const noexcept {
    return static_cast<TObjOwnerParam<T>*>(GetObj())->GetParam();
  }

  static CVParamTransfer Null() noexcept { return CVParamTransfer(); }
};

} // namespace urde
