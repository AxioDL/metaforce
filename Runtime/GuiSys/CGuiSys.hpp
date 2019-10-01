#pragma once

#include <memory>
#include <stack>
#include <unordered_set>
#include <vector>

#include "Runtime/IOStreams.hpp"
#include "Runtime/GuiSys/CSaveableState.hpp"

#include <hecl/hecl.hpp>

namespace urde {
class CGuiFrame;
class CGuiObject;
class CGuiWidget;
class CSimplePool;
class CTextExecuteBuffer;
class CTextParser;
class CVParamTransfer;
class IFactory;
struct SObjectTag;

class CGuiSys {
  friend class CGuiFrame;

public:
  enum class EUsageMode { Zero, One, Two };

private:
  IFactory& x0_resFactory;
  CSimplePool& x4_resStore;
  EUsageMode x8_mode;
  std::unique_ptr<CTextExecuteBuffer> xc_textExecuteBuf;
  std::unique_ptr<CTextParser> x10_textParser;
  std::unordered_set<CGuiFrame*> m_registeredFrames;

  static std::shared_ptr<CGuiWidget> CreateWidgetInGame(FourCC type, CInputStream& in, CGuiFrame* frame,
                                                        CSimplePool* sp);

public:
  CGuiSys(IFactory& resFactory, CSimplePool& resStore, EUsageMode mode);

  CSimplePool& GetResStore() { return x4_resStore; }
  const CSimplePool& GetResStore() const { return x4_resStore; }
  EUsageMode GetUsageMode() const { return x8_mode; }

  void OnViewportResize();
  static void ViewportResizeFrame(CGuiFrame* frame);
};

/** Global GuiSys instance */
extern CGuiSys* g_GuiSys;

/** Global CTextExecuteBuffer instance */
extern CTextExecuteBuffer* g_TextExecuteBuf;

/** Global CTextParser instance */
extern CTextParser* g_TextParser;

} // namespace urde
