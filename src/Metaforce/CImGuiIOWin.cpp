#include "Metaforce/CImGuiIOWin.hpp"

#include "../../extern/sdk/include/dolphin/gx/GXDraw.h"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "imgui.h"

CIOWin::EMessageReturn CImGuiIOWin::OnMessage(const CArchitectureMessage& msg,
                                              CArchitectureQueue&) {
  return kMR_Normal;
}

void CImGuiIOWin::PreDraw() const {}

void CImGuiIOWin::Draw() const {
  if (!gpStateManager || !gpStateManager->GetPlayer()) {
    return;
  }

  // if (ImGui::Begin("Instances")) {
  //   const auto& allObjects = gpStateManager->GetObjectListById(kOL_All);
  //   for (auto it = allObjects.GetFirstObjectIndex(); it != -1;
  //        it = allObjects.GetNextObjectIndex(it)) {
  //     const auto& ent = allObjects[it];
  //     ImGui::Text("%i - %s", ent->GetUniqueId().value, ent->GetDebugName().c_str());
  //   }
  // }
  // ImGui::End();
  //CGraphics::SetModelMatrix(gpStateManager->GetPlayer()->GetTransform());
  //GXDrawSphere(90, 90);
}