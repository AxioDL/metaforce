#pragma once

#include "Metaforce/Input.hpp"

#include <borealis/ui/dropdown_button.hpp>

#include <memory>
#include <vector>

namespace metaforce::ui {

class DeviceDropdown final : public borealis::ui::DropdownButton {
public:
  explicit DeviceDropdown(Rml::Element* parent);
  void update() override;

private:
  DeviceDropdown(Rml::Element* parent,
                 std::shared_ptr< std::vector< input::InputDevice > > devices);
  std::shared_ptr< std::vector< input::InputDevice > > mDevices;
  bool mDevicesDirty = true;
};

} // namespace metaforce::ui
