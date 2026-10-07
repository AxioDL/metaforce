#include "Metaforce/UI/DeviceDropdown.hpp"

#include <borealis/ui/input.hpp>

#include <utility>

namespace metaforce::ui {

namespace {

int ActiveDeviceIndex(const std::vector< input::InputDevice >& devices) {
  const auto activeSource = input::DeviceSource();
  for (size_t index = 0; index < devices.size(); ++index) {
    if (devices[index].source == activeSource) {
      return static_cast< int >(index);
    }
  }
  return 0;
}

void SelectDeviceAt(const std::vector< input::InputDevice >& devices, int index) {
  if (index < 0 || static_cast< size_t >(index) >= devices.size()) {
    return;
  }
  input::SelectDevice(devices[index].source);
}

} // namespace

DeviceDropdown::DeviceDropdown(Rml::Element* parent)
: DeviceDropdown(parent, std::make_shared< std::vector< input::InputDevice > >()) {}

void DeviceDropdown::update() {
  if (!std::exchange(mDevicesDirty, false)) {
    DropdownButton::update();
    return;
  }

  auto devices = input::Devices();

  if (devices != *mDevices) {
    *mDevices = std::move(devices);
    std::vector< Option > options;

    for (const auto& device : *mDevices) {
      options.push_back({device.name, true});
    }

    set_options(std::move(options));
    return;
  }

  DropdownButton::update();
}

DeviceDropdown::DeviceDropdown(Rml::Element* parent, std::shared_ptr< std::vector< input::InputDevice > > devices)
: DropdownButton(parent, {.key = "Input Device", .options = {{"Mouse & Keyboard", true}},
                          .getValue = [devices] { return ActiveDeviceIndex(*devices); },
                          .setValue = [devices](int index) { SelectDeviceAt(*devices, index); }})
, mDevices(std::move(devices))
{
  Component::listen(root()->GetContext()->GetRootElement(),
                    borealis::ui::input::kControllerChangeEvent,
                    [this](Rml::Event&) { mDevicesDirty = true; });
  update();
}

} // namespace metaforce::ui
