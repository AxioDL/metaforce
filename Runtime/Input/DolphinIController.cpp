#include "Runtime/Input/CDolphinController.hpp"

namespace metaforce {
IController* IController::Create() {
  CDolphinController* cont = new CDolphinController();
  cont->Initialize();
  return cont;
}
}