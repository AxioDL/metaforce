#pragma once

#include <memory>

#include "Runtime/Input/IController.hpp"

namespace metaforce {
struct COsContext {
  bool GetOsKeyState(int key) { return false; }
};

class CArchitectureQueue;

namespace WIP {
class CInputGenerator {
  //COsContext& x0_context;
  std::unique_ptr<IController> x4_controller;
  bool x8_ = false;
  bool x9_ = false;
  bool xa_ = false;
  bool xb_ = false;
  float xc_leftDiv;
  float x10_rightDiv;

public:
  CInputGenerator(/*COsContext& context, */float leftDiv, float rightDiv);

  void Update(float dt, CArchitectureQueue& queue);
};

} // namespace WIP
} // namespace metaforce