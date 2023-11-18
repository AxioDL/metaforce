#pragma once

#include <optional>
#include <utility>

#include "Runtime/CScannableObjectInfo.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CQuaternion.hpp>
#include <zeus/CVector2f.hpp>

namespace metaforce {
class CAuiImagePane;
class CGuiFrame;
class CGuiModel;
class CGuiTextPane;
class CGuiWidget;
class CStringTable;
struct CFinalInput;

class CScanDisplay {
  friend class CHudDecoInterfaceScan;

public:
  class CDataDot {
  public:
    enum class EDotState { Hidden, Seek, Hold, RevealPane, Done };

  private:
    EDotState x0_dotState = EDotState::Hidden;
    zeus::CVector2f x4_startPos;
    zeus::CVector2f xc_curPos;
    zeus::CVector2f x14_targetPos;
    float x1c_transDur = 0.f;
    float x20_remTime = 0.f;
    float x24_alpha = 0.f;
    float x28_desiredAlpha = 0.f;

  public:
    explicit CDataDot(const TLockedToken<CTexture>& dataDotTex) {}
    void Update(float dt);
    void Draw(const zeus::CColor& color, float radius);
    float GetTransitionFactor() const { return x1c_transDur > 0.f ? x20_remTime / x1c_transDur : 0.f; }
    void StartTransitionTo(const zeus::CVector2f&, float);
    void SetDestPosition(const zeus::CVector2f&);
    void SetDesiredAlpha(float a) { x28_desiredAlpha = a; }
    void SetDotState(EDotState s) { x0_dotState = s; }
    void SetAlpha(float a) { x24_alpha = a; }
    const zeus::CVector2f& GetCurrPosition() const { return xc_curPos; }
    EDotState GetDotState() const { return x0_dotState; }
  };

  enum class EScanState { Inactive, Downloading, DownloadComplete, ViewingScan, Done };

private:
  TLockedToken<CTexture> x0_dataDot;
  EScanState xc_state = EScanState::Inactive;
  TUniqueId x10_objId = kInvalidUniqueId;
  std::optional<CScannableObjectInfo> x14_scannableInfo;
  const CGuiFrame& xa0_selHud;
  CGuiWidget* xa4_textGroup = nullptr;
  CGuiTextPane* xa8_message = nullptr;
  CGuiTextPane* xac_scrollMessage = nullptr;
  CGuiModel* xb0_xmark = nullptr;
  CGuiModel* xb4_abutton = nullptr;
  CGuiModel* xb8_dash = nullptr;
  rstl::reserved_vector<CDataDot, 4> xbc_dataDots;
  rstl::reserved_vector<std::pair<float, CAuiImagePane*>, 4> x170_paneStates;
  TLockedToken<CStringTable> x194_scanStr; // Used to be optional
  float x1a4_xAlpha = 0.f;
  float x1a8_bodyAlpha = 0.f;
  int x1ac_pageCounter = 0;
  float x1b0_aPulse = 1.f;
  bool x1b4_scanComplete = false;

  float GetDownloadStartTime(size_t idx) const;
  float GetDownloadFraction(size_t idx, float scanningTime) const;
  static void SetScanMessageTypeEffect(CGuiTextPane* pane, bool type);

public:
  explicit CScanDisplay(const CGuiFrame& selHud);
  void ProcessInput(const CFinalInput& input);
  void StartScan(TUniqueId id, const CScannableObjectInfo& scanInfo, CGuiTextPane* message, CGuiTextPane* scrollMessage,
                 CGuiWidget* textGroup, CGuiModel* xmark, CGuiModel* abutton, CGuiModel* dash, float scanTime);
  void StopScan();
  void Update(float dt, float scanningTime);
  void Draw();
  EScanState GetScanState() const { return xc_state; }
  TUniqueId ScanTarget() const { return x10_objId; }
};

} // namespace metaforce
