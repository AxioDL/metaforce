#ifndef _CTWEAKPLAYERCONTROL
#define _CTWEAKPLAYERCONTROL

#include "types.h"

#if VERSION >= VERSION_R3IJ_00

#include "Kyoto/Math/CMayaSpline.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/Tweaks/ITweakObject.hpp"
#include "rstl/reserved_vector.hpp"

class CTweakPlayerControl : public ITweakObject {
public:
  // Descriptor type and field names are inferred from constructors and consumers.
  enum EControlType {
    kCT_None,
    kCT_Physical,
    kCT_PhysicalCombination,
    kCT_Virtual,
    kCT_VirtualCombination,
    kCT_VirtualMenu,
    kCT_Virtual2
  };
  enum EControlBoolean { kCB_And, kCB_Or, kCB_AndNot };
  enum EVirtualMenuShape { kVMS_Annulus, kVMS_Rectangle, kVMS_Sector };

  struct SControlAnnulus {
    SControlAnnulus();
    SControlAnnulus(float centerX, float centerY, float innerRadius, float outerRadius);
    ~SControlAnnulus();
    float mCenterX;
    float mCenterY;
    float mInnerRadius;
    float mOuterRadius;
  };
  struct SControlRectangle {
    SControlRectangle();
    ~SControlRectangle();
    float mCenterX;
    float mCenterY;
    float mWidth;
    float mHeight;
  };
  struct SControlSector {
    SControlSector();
    SControlSector(float centerX, float centerY, float innerRadius, float outerRadius,
                   float centerAngleDegrees, float sweepDegrees);
    ~SControlSector();
    float mCenterX;
    float mCenterY;
    float mInnerRadius;
    float mOuterRadius;
    float mCenterAngleDegrees;
    float mSweepDegrees;
  };
  struct SPhysicalControl {
    SPhysicalControl();
    SPhysicalControl(CFinalInput::EPhysicalControl control, const CMayaSpline& response);
    ~SPhysicalControl();
    CFinalInput::EPhysicalControl mControl;
    CMayaSpline mResponse;
  };
  struct SVirtualMenu {
    SVirtualMenu();
    SVirtualMenu(EVirtualMenuShape shape, const SControlAnnulus& annulus,
                 const SControlRectangle& rectangle, const SControlSector& sector);
    ~SVirtualMenu();
    EVirtualMenuShape mShape;
    SControlAnnulus mAnnulus;
    SControlRectangle mRectangle;
    SControlSector mSector;
  };
  struct SCommandDescription {
    SCommandDescription(const SCommandDescription& other);
    SCommandDescription(CControlMapper::ECommands command, EControlType type,
                        const SPhysicalControl& physical);
    SCommandDescription(CControlMapper::ECommands command, EControlType type,
                        CFinalInput::EMotionControl motion);
    SCommandDescription(CControlMapper::ECommands command, EControlType type,
                        const SPhysicalControl& primary, EControlBoolean operation,
                        const SPhysicalControl& secondary);
    SCommandDescription(CControlMapper::ECommands command, EControlType type,
                        const SVirtualMenu& menu);
    ~SCommandDescription();
    CControlMapper::ECommands mCommand;
    EControlType mType;
    SPhysicalControl mPrimary;
    EControlBoolean mPhysicalBoolean;
    SPhysicalControl mSecondary;
    CFinalInput::EMotionControl mPrimaryMotion;
    EControlBoolean mVirtualBoolean;
    CFinalInput::EMotionControl mSecondaryMotion;
    CFinalInput::ESwingControl mSwing;
    SVirtualMenu mVirtualMenu;
  };

  explicit CTweakPlayerControl(uint controlPreset);
  ~CTweakPlayerControl() override;
  const SCommandDescription& GetCommandDescription(CControlMapper::ECommands command) const;
  CControlMapper::SCommandMapping GetCommandMapping(CControlMapper::ECommands command) const;
  const CMayaSpline& GetTurnLeftResponse() const;
  const CMayaSpline& GetTurnRightResponse() const;
  const CMayaSpline& GetCursorUpResponse() const { return mResponseCurves[4]; }
  const CMayaSpline& GetCursorDownResponse() const { return mResponseCurves[5]; }
  const CMayaSpline& GetCursorRightResponse() const { return mResponseCurves[6]; }
  const CMayaSpline& GetCursorLeftResponse() const { return mResponseCurves[7]; }
  const CMayaSpline& GetHeldCursorUpResponse() const { return mResponseCurves[8]; }
  const CMayaSpline& GetHeldCursorDownResponse() const { return mResponseCurves[9]; }
  const CMayaSpline& GetBallCursorHorizontalResponse() const { return mResponseCurves[10]; }
  const CMayaSpline& GetBallCursorVerticalResponse() const { return mResponseCurves[11]; }

private:
  CControlMapper::SCommandMapping
  GetMappingFromDescription(const SCommandDescription& description) const;
  void InitializeControls();

  rstl::reserved_vector< CMayaSpline, 16 > mResponseCurves;
  uint mControlPreset;
  rstl::reserved_vector< SCommandDescription, 91 > mCommands;
};
CHECK_SIZEOF(CTweakPlayerControl, 0x53b0)

#else

#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "MetroidPrime/CControlMapper.hpp"

#include "Kyoto/TOneStatic.hpp"

#include "rstl/reserved_vector.hpp"

class CInputStream;
class CTweakPlayerControl;

class CTweakPlayerControl : public ITweakObject {
public:
  CTweakPlayerControl(CInputStream&);
  ~CTweakPlayerControl() override;

  ControlMapper::EFunctionList GetMapping(ControlMapper::ECommands command) const;

private:
  rstl::reserved_vector< ControlMapper::EFunctionList, 67 > m_mappings;
};

#endif

#endif // _CTWEAKPLAYERCONTROL
