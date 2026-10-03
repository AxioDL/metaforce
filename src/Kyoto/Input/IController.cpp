#include "Kyoto/Input/IController.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Input/CWiiInput.hpp"

IController::IController() {}

IController::~IController() {}

IController* IController::Create(const COsContext& ctx) { return rs_new CWiiInput(); }
