#include "Metaforce/CImGuiIOWin.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "aurora/gfx.h"
#include "imgui.h"

#include <fmt/format.h>
#include <string>

CIOWin::EMessageReturn CImGuiIOWin::OnMessage(const CArchitectureMessage& msg,
                                              CArchitectureQueue&) {
  return kMR_Normal;
}

void CImGuiIOWin::PreDraw() const {}

void CImGuiIOWin::Draw() const {
  if (!gpStateManager || !gpStateManager->GetPlayer()) {
    return;
  }

  ShowDebugOverlay();
}

void CImGuiIOWin::ShowDebugOverlay() const {
  if (!gpStateManager || !gpStateManager->GetPlayer()) {
    return;
  }
  const CPlayer& pl = *gpStateManager->GetPlayer();
  const CQuaternion plQ = CQuaternion::FromMatrix(pl.GetTransform().GetRotation().BuildMatrix3f());
  const CTransform4f camXf =
      gpStateManager->GetCameraManager()->GetCurrentCameraTransform(*gpStateManager);
  const CQuaternion camQ = CQuaternion::FromMatrix(camXf.GetRotation().BuildMatrix3f());

  const auto* stats = aurora_get_stats();
  ImGuiIO& io = ImGui::GetIO();
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
  // if (m_debugOverlayCorner != -1) {
  //   SetOverlayWindowLocation(m_debugOverlayCorner);
  //   windowFlags |= ImGuiWindowFlags_NoMove;
  // }
  ImGui::SetNextWindowBgAlpha(0.65f);
  if (ImGui::Begin("Debug Overlay", nullptr, windowFlags)) {
    std::string playerStats = fmt::format(
    "Player Position x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
    "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n"
    "       Momentum x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
    "       Velocity x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
    "Camera Position x: {: .2f}, y: {: .2f}, z {: .2f}\n"
    "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n",
    pl.GetTranslation().GetX(), pl.GetTranslation().GetY(), pl.GetTranslation().GetZ(),
    CMath::Rad2Deg(plQ.AxisX()), CMath::Rad2Deg(plQ.AxisY()), CMath::Rad2Deg(plQ.AxisZ()),
    pl.GetMomentumWR().GetX(), pl.GetMomentumWR().GetY(), pl.GetMomentumWR().GetZ(),
    pl.GetVelocityWR().GetX(), pl.GetVelocityWR().GetY(), pl.GetVelocityWR().GetZ(),
    camXf.GetTranslation().GetX(), camXf.GetTranslation().GetY(), camXf.GetTranslation().GetZ(),
    CMath::Rad2Deg(camQ.AxisX()), CMath::Rad2Deg(camQ.AxisY()), CMath::Rad2Deg(camQ.AxisZ()));
    
    ImGui::Text("%s", playerStats.c_str());
  }
  ImGui::End();
  // const CPlayer& pl = g_StateManager->GetPlayer();
  //  const zeus::CQuaternion plQ =
  //      zeus::CQuaternion(pl.GetTransform().GetRotation().BuildMatrix3f());
  //  const zeus::CTransform4f camXf =
  //      g_StateManager->GetCameraManager()->GetCurrentCameraTransform(*g_StateManager);
  //  const zeus::CQuaternion camQ = zeus::CQuaternion(camXf.GetRotation().BuildMatrix3f());
  //  ImGuiStringViewText(fmt::format(
  //      "Player Position x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
  //      "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n"
  //      "       Momentum x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
  //      "       Velocity x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
  //      "Camera Position x: {: .2f}, y: {: .2f}, z {: .2f}\n"
  //      "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n",
  //      pl.GetTranslation().x(), pl.GetTranslation().y(), pl.GetTranslation().z(),
  //      zeus::radToDeg(plQ.roll()), zeus::radToDeg(plQ.pitch()), zeus::radToDeg(plQ.yaw()),
  //      pl.GetMomentum().x(), pl.GetMomentum().y(), pl.GetMomentum().z(), pl.GetVelocity().x(),
  //      pl.GetVelocity().y(), pl.GetVelocity().z(), camXf.origin.x(), camXf.origin.y(),
  //      camXf.origin.z(), zeus::radToDeg(camQ.roll()), zeus::radToDeg(camQ.pitch()),
  //      zeus::radToDeg(camQ.yaw())));
#if 0
  const std::array flags{
      m_frameCounter && (g_StateManager != nullptr),
      m_frameRate,
      m_inGameTime && (g_StateManager != nullptr),
      m_roomTimer && (g_StateManager != nullptr),
      m_playerInfo && (g_StateManager != nullptr) && (g_StateManager->Player() != nullptr),
      m_worldInfo && (g_StateManager != nullptr) && m_developer,
      m_areaInfo && (g_StateManager != nullptr) && m_developer,
      m_layerInfo && (g_StateManager != nullptr) && m_developer,
      m_randomStats && m_developer,
      m_drawCallInfo && m_developer,
      m_bufferInfo && m_developer,
      m_pipelineInfo && m_developer,
      m_resourceStats && (g_SimplePool != nullptr),
  };

  if (std::ranges::all_of(flags, [](const bool v) { return !v; })) {
    return;
  }

  const auto* stats = aurora_get_stats();
  ImGuiIO& io = ImGui::GetIO();
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
  if (m_debugOverlayCorner != -1) {
    SetOverlayWindowLocation(m_debugOverlayCorner);
    windowFlags |= ImGuiWindowFlags_NoMove;
  }
  ImGui::SetNextWindowBgAlpha(0.65f);
  if (ImGui::Begin("Debug Overlay", nullptr, windowFlags)) {
    bool hasPrevious = false;
    if (m_frameCounter && g_StateManager != nullptr) {
      ImGuiStringViewText(fmt::format("Frame: {}\n", g_StateManager->GetUpdateFrameIndex()));
      hasPrevious = true;
    }
    if (m_frameRate) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("FPS: {:.1f}\n", io.Framerate));
    }
    if (m_inGameTime && g_GameState != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      double igt = g_GameState->GetTotalPlayTime();
      u32 ms = u64(igt * 1000) % 1000;
      auto pt = std::div(int(igt), 3600);
      ImGuiStringViewText(fmt::format("Play Time: {:02d}:{:02d}:{:02d}.{:03d}\n", pt.quot,
                                      pt.rem / 60, pt.rem % 60, ms));
    }
    if (m_roomTimer && g_StateManager != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      double igt = g_GameState->GetTotalPlayTime();
      double currentRoomTime = igt - m_currentRoomStart;
      u32 curFrames = u32(std::round(u32(currentRoomTime * 60)));
      u32 lastFrames = u32(std::round(u32(m_lastRoomTime * 60)));
      ImGuiStringViewText(fmt::format("Room Time: {:7.3f} / {:5d} | Last Room:{:7.3f} / {:5d}\n",
                                      currentRoomTime, curFrames, m_lastRoomTime, lastFrames));
    }
    if (m_playerInfo && g_StateManager != nullptr && g_StateManager->Player() != nullptr &&
        m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      const CPlayer& pl = g_StateManager->GetPlayer();
      const zeus::CQuaternion plQ =
          zeus::CQuaternion(pl.GetTransform().GetRotation().BuildMatrix3f());
      const zeus::CTransform4f camXf =
          g_StateManager->GetCameraManager()->GetCurrentCameraTransform(*g_StateManager);
      const zeus::CQuaternion camQ = zeus::CQuaternion(camXf.GetRotation().BuildMatrix3f());
      ImGuiStringViewText(fmt::format(
          "Player Position x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
          "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n"
          "       Momentum x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
          "       Velocity x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
          "Camera Position x: {: .2f}, y: {: .2f}, z {: .2f}\n"
          "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n",
          pl.GetTranslation().x(), pl.GetTranslation().y(), pl.GetTranslation().z(),
          zeus::radToDeg(plQ.roll()), zeus::radToDeg(plQ.pitch()), zeus::radToDeg(plQ.yaw()),
          pl.GetMomentum().x(), pl.GetMomentum().y(), pl.GetMomentum().z(), pl.GetVelocity().x(),
          pl.GetVelocity().y(), pl.GetVelocity().z(), camXf.origin.x(), camXf.origin.y(),
          camXf.origin.z(), zeus::radToDeg(camQ.roll()), zeus::radToDeg(camQ.pitch()),
          zeus::radToDeg(camQ.yaw())));
    }
    if (m_worldInfo && g_StateManager != nullptr && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      const std::string name =
          ImGuiLoadStringTable(g_StateManager->GetWorld()->IGetStringTableAssetId(), 0);
      ImGuiStringViewText(fmt::format("World Asset ID: 0x{}, Name: {}\n",
                                      g_GameState->CurrentWorldAssetId(), name));
    }
    if (m_areaInfo && g_StateManager != nullptr && m_developer) {
      const metaforce::TAreaId aId = g_GameState->CurrentWorldState().GetCurrentAreaId();
      if (g_StateManager->GetWorld() != nullptr && g_StateManager->GetWorld()->DoesAreaExist(aId)) {
        if (hasPrevious) {
          ImGui::Separator();
        }
        hasPrevious = true;

        const auto& layerStates = g_GameState->CurrentWorldState().GetLayerState();
        std::string layerBits;
        u32 totalActive = 0;
        for (int i = 0; i < layerStates->GetAreaLayerCount(aId); ++i) {
          if (layerStates->IsLayerActive(aId, i)) {
            ++totalActive;
            layerBits += "1";
          } else {
            layerBits += "0";
          }
        }
        CGameArea* pArea = g_StateManager->GetWorld()->GetArea(aId);
        CAssetId stringId = pArea->IGetStringTableAssetId();
        ImGuiStringViewText(
            fmt::format("Area Asset ID: 0x{}, Name: {}\nArea ID: {}, Active Layer bits: {}\n",
                        pArea->GetAreaAssetId(), ImGuiLoadStringTable(stringId, 0),
                        pArea->GetAreaId(), layerBits));
      }
    }
    if (m_layerInfo && g_StateManager != nullptr && m_developer) {
      const metaforce::TAreaId aId = g_GameState->CurrentWorldState().GetCurrentAreaId();
      const auto* world = g_StateManager->GetWorld();
      if (world != nullptr && world->DoesAreaExist(aId) && world->GetWorldLayers()) {
        if (hasPrevious) {
          ImGui::Separator();
        }
        hasPrevious = true;

        ImGuiStringViewText("Area Layers:");

        ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        ImVec4 inactiveColor = activeColor;
        inactiveColor.w = 0.5f;

        const CWorldLayers& layers = world->GetWorldLayers().value();
        const auto& layerStates = g_GameState->CurrentWorldState().GetLayerState();
        int layerCount = int(layerStates->GetAreaLayerCount(aId));
        u32 startNameIdx = layers.m_areas[aId].m_startNameIdx;
        if (startNameIdx + layerCount > layers.m_names.size()) {
          ImGui::Text("Broken layer data, please re-package");
        } else {
          for (int i = 0; i < layerCount; ++i) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  layerStates->IsLayerActive(aId, i) ? activeColor : inactiveColor);
            ImGuiStringViewText("  " + layers.m_names[startNameIdx + i]);
            ImGui::PopStyleColor();
          }
        }
      }
    }
    if (m_randomStats && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(
          fmt::format("CRandom16::Next calls: {}\n", metaforce::CRandom16::GetNumNextCalls()));
      ImGuiStringViewText(fmt::format("CRandom16::LastSeed: 0x{:08X}\n", CRandom16::GetLastSeed()));
    }
    if (m_resourceStats && g_SimplePool != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("Resource Objects: {}\n", g_SimplePool->GetLiveObjects()));
    }
    if (m_pipelineInfo && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("Queued pipelines:  {}\n", stats->queuedPipelines));
      ImGuiStringViewText(fmt::format("Done pipelines:    {}\n", stats->createdPipelines));
    }
    if (m_drawCallInfo && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("Draw call count:   {}\n", stats->drawCallCount));
      ImGuiStringViewText(fmt::format("Merged draw calls: {}\n", stats->mergedDrawCallCount));
    }
    if (m_bufferInfo && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(
          fmt::format("Vertex size:       {}\n", BytesToString(stats->lastVertSize)));
      ImGuiStringViewText(
          fmt::format("Uniform size:      {}\n", BytesToString(stats->lastUniformSize)));
      ImGuiStringViewText(
          fmt::format("Index size:        {}\n", BytesToString(stats->lastIndexSize)));
      ImGuiStringViewText(
          fmt::format("Storage size:      {}\n", BytesToString(stats->lastStorageSize)));
      ImGuiStringViewText(
          fmt::format("Tex upload size:   {}\n", BytesToString(stats->lastTextureUploadSize)));
      ImGuiStringViewText(fmt::format("Total:             {}\n",
                                      BytesToString(stats->lastVertSize + stats->lastUniformSize +
                                                    stats->lastIndexSize + stats->lastStorageSize +
                                                    stats->lastTextureUploadSize)));
    }
    if (ShowCornerContextMenu(m_debugOverlayCorner, m_inputOverlayCorner)) {
      m_cvarCommons.m_debugOverlayCorner->fromInteger(m_debugOverlayCorner);
    }
  }
  ImGui::End();
#endif
}