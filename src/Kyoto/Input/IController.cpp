#include "Kyoto/Input/IController.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Input/CRevolutionController.hpp"

IController::IController() {}

IController::~IController() {}

IController* IController::Create(const COsContext& ctx) { return rs_new CRevolutionController(); }
