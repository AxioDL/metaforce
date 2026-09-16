#ifndef _CFONTRENDERSTATE
#define _CFONTRENDERSTATE

#include "rstl/list.hpp"

#include "Kyoto/Text/CBlockInstruction.hpp"
#include "Kyoto/Text/CDrawStringOptions.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CSaveableState.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#include <rstl/vector.hpp>

class CBlockInstruction;
class CLineInstruction;
class CFontRenderState {
public:
  CFontRenderState();
  void RefreshColor(EColorType col);
  uint ConvertToTextureSpace(const CTextColor& color) const;
  void PushState();
  void PopState();
  void SetColor(EColorType type, const CTextColor& color);
  void RefreshPalette();
  TToken< CRasterFont >& GetFont() { return *x0_state.GetFont(); }
  bool IsFinishedLoading() { return x0_state.IsFinishedLoading(); }
  CDrawStringOptions& GetOptions() { return x0_state.GetOptions(); }
  void SetFont(const TToken< CRasterFont >& font) { x0_state.SetFont(font); }
  rstl::vector< CTextColor >& GetColors() { return x0_state.GetColors(); }
  rstl::vector< bool >& GetOverride() { return x0_state.GetOverride(); }
  float GetLineSpacing() const { return x0_state.GetLineSpacing(); }
  void SetLineSpacing(float spacing) { x0_state.SetLineSpacing(spacing); }
  int GetLineExtraSpacing() const { return x0_state.GetLineExtraSpacing(); }
  void SetExtraLineSpace(int spacing) { x0_state.SetLineExtraSpace(spacing); }
  const CBlockInstruction* GetBlock() const { return x88_curBlock; }
  void SetBlock(const CBlockInstruction* block) {
    x88_curBlock = const_cast< CBlockInstruction* >(block);
  }
  void SetX(int x) { xd4_curX = x; }
  int GetX() const { return xd4_curX; }
  void SetY(int y) { xd8_curY = y; }
  int GetY() const { return xd8_curY; }
  const CLineInstruction* GetLine() const { return xdc_currentLineInst; }
  bool IsFirstWordOnLine() const { return x108_lineInitialized; }
  void SetFirstWordOnLine(bool v) { x108_lineInitialized = v; }

  void SetLine(const CLineInstruction* line) { xdc_currentLineInst = line; }

  void SubX(const int x) { xd4_curX -= x; }

  void AddX(const int x) { xd4_curX += x; }

  void SubY(const int y) { xd8_curY -= y; }
  void AddY(const int y) { xd8_curY += y; }

  int GetSpacing(const int v) const {
    if (GetBlock()->GetVerticalJustification() == kVerticalJustification_Full) {
      return v;
    }

    return (static_cast< int >(static_cast< float >(v) * GetLineSpacing()) + GetLineExtraSpacing());
  }

private:
  CSaveableState x0_state;
  CBlockInstruction* x88_curBlock;
  CDrawStringOptions x8c_drawOpts;
  int xd4_curX;
  int xd8_curY;
  const CLineInstruction* xdc_currentLineInst;
  uint xe0_;
  uint xe4_;
  rstl::vector< uint > xe8_;
  rstl::vector< uchar > xf8_;
  bool x108_lineInitialized;
  rstl::list< CSaveableState > x10c_pushedStates;
};

CHECK_SIZEOF(CFontRenderState,
             (VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02 ? 0x12c : 0x124))

#endif // _CFONTRENDERSTATE
