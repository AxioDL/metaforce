#ifndef _CHUDBALLINTERFACE
#define _CHUDBALLINTERFACE

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/reserved_vector.hpp"

class CGuiCamera;
class CGuiFrame;
class CGuiModel;
class CGuiTextPane;
class CGuiWidget;

class CHudBallInterface {
public:
  CHudBallInterface(CGuiFrame& selHud, int pbAmount, int pbCapacity, int availableBombs,
                    bool hasBombs, bool hasPb);
  void SetBombParams(int pbAmount, int pbCapacity, int availableBombs, bool hasBombs, bool hasPb,
                     bool init);
  void SetBallModeFactor(float t);

private:
  CGuiCamera* mCamera;
  CGuiWidget* mBasewidget_bombstuff;
  CGuiWidget* mBasewidget_bombdeco;
  CGuiModel* mModel_bombicon;
  CGuiTextPane* mTextpane_bombdigits;
  rstl::reserved_vector< CGuiWidget*, 3 > mGroup_bombfilled;
  rstl::reserved_vector< CGuiWidget*, 3 > mGroup_bombempty;
  CVector3f mCamPos;
  int mPbAmount;
  int mPbCapacity;
  int mAvailableBombs;
  bool mHasPb;
  void UpdatePowerBombReadoutColors();
};
CHECK_SIZEOF(CHudBallInterface, 0x50)

#endif // _CHUDBALLINTERFACE
