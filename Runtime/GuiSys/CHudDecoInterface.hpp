#ifndef __URDE_CHUDDECOINTERFACE_HPP__
#define __URDE_CHUDDECOINTERFACE_HPP__

#include "zeus/CMatrix3f.hpp"
#include "zeus/CQuaternion.hpp"
#include "zeus/CVector3f.hpp"
#include "CToken.hpp"
#include "CScanDisplay.hpp"
#include "World/CPlayer.hpp"

namespace urde
{
class CGuiFrame;
class CFinalInput;
class CStateManager;
class CGuiCamera;
class CGuiWidget;
class CAuiEnergyBarT01;
class CGuiModel;

class IHudDecoInterface
{
public:
    virtual void SetIsVisibleDebug(bool v)=0;
    virtual void SetIsVisibleGame(bool v)=0;
    virtual void SetHudRotation(const zeus::CQuaternion& rot)=0;
    virtual void SetHudOffset(const zeus::CVector3f& off)=0;
    virtual void SetReticuleTransform(const zeus::CMatrix3f& xf) {}
    virtual void UpdateTransforms() {}
    virtual void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position)=0;
    virtual void SetFrameColorValue(float v);
    virtual void Update(float dt, const CStateManager& stateMgr)=0;
    virtual void Draw() const {}
    virtual void ProcessInput(const CFinalInput& input) {}
    virtual void SetCameraParms(float fov, float y, float z)=0;
    virtual void UpdateHudAlpha()=0;
    virtual float GetMessageTextAlpha() const { return 1.f; }
    virtual ~IHudDecoInterface() = default;
};

class CHudDecoInterfaceCombat : public IHudDecoInterface
{
    zeus::CQuaternion x4_rotation;
    zeus::CVector3f x14_pivotPosition;
    zeus::CVector3f x20_offset;
    zeus::CVector3f x2c_camPos;
    zeus::CVector3f x38_basePosition;
    zeus::CMatrix3f x44_baseRotation;
    bool x68_24_visDebug : 1;
    bool x68_25_visGame : 1;
    CGuiCamera* x6c_camera;
    CGuiWidget* x70_basewidget_pivot;
    CGuiWidget* x74_basewidget_deco;
    CGuiWidget* x78_basewidget_tickdeco0;
    CGuiWidget* x7c_basewidget_frame;
    void UpdateVisibility();
public:
    CHudDecoInterfaceCombat(CGuiFrame& selHud);
    void SetIsVisibleDebug(bool v);
    void SetIsVisibleGame(bool v);
    void SetHudRotation(const zeus::CQuaternion& rot);
    void SetHudOffset(const zeus::CVector3f& off);
    void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position);
    void SetFrameColorValue(float v);
    void Update(float dt, const CStateManager& stateMgr);
    void SetCameraParms(float fov, float y, float z);
    void UpdateHudAlpha();
};

class CHudDecoInterfaceScan : public IHudDecoInterface
{
    TLockedToken<CGuiFrame> x4_scanHudFlat;
    CGuiFrame* x10_loadedScanHudFlat = nullptr;
    CGuiFrame& x14_selHud;
    CScanDisplay x18_scanDisplay;
    TUniqueId x1d0_latestHudPoi = kInvalidUniqueId;
    TUniqueId x1d2_latestScanningObject = kInvalidUniqueId;
    CPlayer::EPlayerScanState x1d4_latestScanState = CPlayer::EPlayerScanState::Zero;
    float x1d8_scanningTime = 0.f;
    float x1dc_ = 0.f;
    float x1e0_ = 1.f;
    zeus::CQuaternion x1e4_rotation;
    zeus::CVector3f x1f4_pivotPosition;
    zeus::CVector3f x200_offset;
    zeus::CVector3f x20c_camPos;
    zeus::CVector3f x218_leftsidePosition;
    zeus::CVector3f x224_rightsidePosition;
    float x230_sidesTimer = 0.f;
    float x234_sidesPositioner;
    float x238_scanningTextAlpha = 0.f;
    float x23c_scanBarAlpha = 0.f;
    bool x240_24_visDebug : 1;
    bool x240_25_visGame : 1;
    CGuiCamera* x244_camera;
    CGuiWidget* x248_basewidget_pivot;
    CGuiWidget* x24c_basewidget_leftside;
    CGuiWidget* x250_basewidget_rightside;
    CGuiTextPane* x254_flat_textpane_scanning;
    CGuiWidget* x258_flat_basewidget_scanguage;
    CAuiEnergyBarT01* x25c_flat_energybart01_scanbar;
    CGuiWidget* x260_flat_basewidget_textgroup;
    CGuiTextPane* x264_flat_textpane_message;
    CGuiTextPane* x268_flat_textpane_scrollmessage;
    CGuiModel* x26c_flat_model_xmark;
    CGuiModel* x270_flat_model_abutton;
    CGuiModel* x274_flat_model_dash;
    void UpdateVisibility();
public:
    CHudDecoInterfaceScan(CGuiFrame& selHud);
    void SetIsVisibleDebug(bool v);
    void SetIsVisibleGame(bool v);
    void SetHudRotation(const zeus::CQuaternion& rot);
    void SetHudOffset(const zeus::CVector3f& off);
    void SetReticuleTransform(const zeus::CMatrix3f& xf);
    void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position);
    void SetFrameColorValue(float v);
    void InitializeFlatFrame();
    const CScannableObjectInfo* GetCurrScanInfo(const CStateManager& stateMgr) const;
    void UpdateScanDisplay(const CStateManager& stateMgr, float dt);
    void Update(float dt, const CStateManager& stateMgr);
    void Draw() const;
    void ProcessInput(const CFinalInput& input);
    void SetCameraParms(float fov, float y, float z);
    void UpdateHudAlpha();
    float GetMessageTextAlpha() const;
};

class CHudDecoInterfaceXRay : public IHudDecoInterface
{
    void UpdateVisibility();
public:
    CHudDecoInterfaceXRay(CGuiFrame& selHud);
    void SetIsVisibleDebug(bool v);
    void SetIsVisibleGame(bool v);
    void SetHudRotation(const zeus::CQuaternion& rot);
    void SetHudOffset(const zeus::CVector3f& off);
    void SetReticuleTransform(const zeus::CMatrix3f& xf);
    void UpdateTransforms();
    void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position);
    void SetFrameColorValue(float v);
    void Update(float dt, const CStateManager& stateMgr);
    void SetCameraParms(float fov, float y, float z);
    void UpdateHudAlpha();
};

class CHudDecoInterfaceThermal : public IHudDecoInterface
{
    void UpdateVisibility();
public:
    CHudDecoInterfaceThermal(CGuiFrame& selHud);
    void SetIsVisibleDebug(bool v);
    void SetIsVisibleGame(bool v);
    void SetHudRotation(const zeus::CQuaternion& rot);
    void SetHudOffset(const zeus::CVector3f& off);
    void SetReticuleTransform(const zeus::CMatrix3f& xf);
    void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position);
    void Update(float dt, const CStateManager& stateMgr);
    void SetCameraParms(float fov, float y, float z);
    void UpdateHudAlpha();
};

}

#endif // __URDE_CHUDDECOINTERFACE_HPP__
