#ifndef _CSLIDESHOW
#define _CSLIDESHOW

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CDependencyGroup;
class CFinalInput;
class CGuiTextSupport;
class CModel;
class CStaticAudioPlayer;
class CStringTable;
class CTexture;

class CSlideShow : public CIOWin {
public:
  ~CSlideShow() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  bool GetIsContinueDraw() const override;
  void Draw() const override;

  CSlideShow();

  static uint SlideShowGalleryFlags();

private:
#if VERSION >= VERSION_GM8P_00
  struct SGalleryData {
    int mGallery;
    rstl::vector< const SObjectTag* > mTextures;
    rstl::vector< rstl::pair< int, int > > mSlides;

    explicit SGalleryData(int gallery) : mGallery(gallery) {}
  };

  struct STexture {
    rstl::auto_ptr< TToken< CTexture > > mToken;
    CVector2f mLeftBottom;
    CVector2f mRightTop;
    mutable float mAlpha;

    STexture() : mLeftBottom(0.f, 0.f), mRightTop(0.f, 0.f), mAlpha(0.f) {}
  };
#else
  typedef rstl::pair< int, rstl::vector< const SObjectTag* > > SGalleryData;
#endif
  struct SSlideData {
    CSlideShow* mParent;
    int mGallery;
    int mSlide;
#if VERSION >= VERSION_GM8P_00
    rstl::vector< STexture > mTextures;
    int mColumns;
    float mTextureWidth;
    float mTextureHeight;
    bool mReady;
    bool mStopLoading;
#else
    rstl::auto_ptr< TToken< CTexture > > mTexture;
    bool mReady;
#endif
    CVector2f mVpOffset;
    CVector2f mVpSize;
    CVector2f mCanvasSize;
    CColor mMulColor;

    SSlideData()
    : mParent(nullptr)
    , mGallery(-1)
    , mSlide(-1)
#if VERSION >= VERSION_GM8P_00
    , mColumns(0)
#endif
    , mReady(false)
#if VERSION >= VERSION_GM8P_00
    , mStopLoading(false)
#endif
    , mVpOffset(0.f, 0.f)
    , mVpSize(0.f, 0.f)
    , mCanvasSize(0.f, 0.f)
#if VERSION >= VERSION_GM8P_00
    , mMulColor(CColor::White().WithAlphaOf(0.f)) {}
#else
    , mMulColor(CColor::White()) {
      mMulColor.SetAlpha(0.f);
    }
#endif

#if VERSION >= VERSION_GM8P_00
    const bool IsLoaded() const;
#else
    bool IsLoaded() const { return !mTexture.null() && mTexture->IsLoaded(); }
#endif
    bool IsReady() const { return IsLoaded() && mReady; }
    void Reset();
    void Draw() const;
    void InitializeViewport();
    EMessageReturn ProcessUserInput(const CFinalInput& input);
  };

  friend struct SSlideData;

  bool LoadTXTRDep(const char* name);
  void BuildGalleryLists(uint flags);
  EMessageReturn ProcessUserInput(const CFinalInput& input);
  EMessageReturn AdvanceSlide(bool forward);
  void LoadSlide();
  bool IsControlsAnimating() const;
  void SetShowControls(bool show);
  float GetControlsTextHeight() const;
  float GetControlsHeight() const;
  void UpdateControls(float dt);
  void UpdateControlsText(const CFinalInput& input);
  void UpdateSlideNumber(float dt);
  void DrawSlideNumber() const;
  void DrawControls() const;
  void DrawControlsBorder() const;
  bool AreAllDepsLoaded(const rstl::vector< TToken< CDependencyGroup > >& deps) const;
  void SetDependenciesLocked(rstl::vector< TToken< CDependencyGroup > >& deps, bool locked);
  void SetTexturesLocked(rstl::vector< CToken >& textures, bool locked);
  void UpdateMusicVolume(float time);
  void SetZoomSfx(bool active);
  void SetPanSfx(bool active);

  int mPhase;
  rstl::vector< TToken< CDependencyGroup > > mGalleryTXTRDeps;
  rstl::vector< SGalleryData > mGalleries;
  rstl::single_ptr< TToken< CModel > > mGalleryBorder;
  int x3c_;
  int mTotalSlides;
  int mGallery;
  int mSlide;
  float mCrossfadeTimer;
  float mRepeatTimer;
  float mIdleTimer;
  float mSlideNumberTimer;
  SSlideData mSlideA;
  SSlideData mSlideB;
  rstl::single_ptr< CGuiTextSupport > mControlsText;
#if VERSION >= VERSION_GM8P_00
  rstl::single_ptr< CGuiTextSupport > mGalleryNameText;
#endif
  rstl::single_ptr< CGuiTextSupport > mSlideNumberText;
  rstl::single_ptr< CStaticAudioPlayer > mAudio;
#if VERSION >= VERSION_GM8P_00
  TToken< CStringTable > mGalleryNames;
  rstl::vector< rstl::wstring > mGalleryLabels;
#else
  rstl::vector< rstl::auto_ptr< TToken< CTexture > > > mGalleryTags;
#endif
  CSfxHandle mPanSfx;
  CSfxHandle mZoomSfx;
  int mLStick;
  int mCStick;
  int mLTrigger;
  int mRTrigger;
  rstl::vector< CToken > mStickTextures;
  rstl::vector< CToken > mButtonTextures;
  rstl::vector< CToken > mTextures;
  float mControlsOffset;
  float mSlideNumberOffset;
  float mFadeTimer;
  bool mShowControls : 1;
  bool mControlsFadeIn : 1;
  bool mControlsFadeOut : 1;
  bool x134_27_ : 1;
  bool mDisableInput : 1;
  bool mExit : 1;
  bool mIntroFade : 1;
  bool mOutroFade : 1;
  bool mGalleryChanged : 1;
};
CHECK_SIZEOF(CSlideShow, (VERSION >= VERSION_GM8P_00 ? 0x16c : 0x138))

#endif // _CSLIDESHOW
