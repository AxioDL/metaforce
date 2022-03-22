#pragma once

#include <memory>

#include "Runtime/Input/IController.hpp"
#include "Runtime/Input/CFinalInput.hpp"

namespace metaforce {
struct COsContext {
  bool GetOsKeyState(int key) { return false; }
};

class CArchitectureQueue;

class CInputGenerator {
  // COsContext& x0_context;
  std::unique_ptr<IController> x4_controller;
  std::array<bool, 4> x8_connectedControllers{};
  float xc_leftDiv;
  float x10_rightDiv;

  CFinalInput m_lastInput;

public:
  CInputGenerator(/*COsContext& context, */ float leftDiv, float rightDiv);

  void Update(float dt, CArchitectureQueue& queue);

  IController* GetController() const { return x4_controller.get(); }
  CFinalInput GetLastInput() const { return m_lastInput; }
};
} // namespace metaforce