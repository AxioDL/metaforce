#include "Hacks.hpp"

#include "dawn/native/Device.h"
#ifdef _WIN32
#include "dawn/native/d3d12/AdapterD3D12.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#endif

namespace dawn::native {
class HackedDevice : public DeviceBase {
public:
  void _ForceSetToggle(Toggle toggle, bool isEnabled) { ForceSetToggle(toggle, isEnabled); }
};
} // namespace dawn::native

namespace aurora::gpu::hacks {
void apply_toggles(WGPUDevice device) {
  auto* hack = static_cast<dawn::native::HackedDevice*>(static_cast<void*>(device));
  hack->_ForceSetToggle(dawn::native::Toggle::UseUserDefinedLabelsInBackend, true);
#if _WIN32
  hack->_ForceSetToggle(dawn::native::Toggle::UseDXC, true);
  auto* backend = dawn::native::d3d12::ToBackend(hack->GetAdapter())->GetBackend();
  backend->EnsureDxcCompiler();
  backend->EnsureDxcLibrary();
  backend->EnsureDxcValidator();
#endif
}
} // namespace aurora::gpu::hacks
