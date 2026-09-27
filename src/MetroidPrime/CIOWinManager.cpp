#include "MetroidPrime/CIOWinManager.hpp"

#include "MetroidPrime/CIOWin.hpp"
#include "MetroidPrime/Decode.hpp"

#include "rstl/string.hpp"

static const char* tmp = "Msg[%3d] %s";
static const char* tmp2 = "-------------------------";
static const char* tmp3 = "Draw[%3d] %s";

CIOWinManager::CIOWinManager()
: mDrawRoot(nullptr), mPumpRoot(nullptr), mLocalGatherQueue() {}

CIOWinManager::~CIOWinManager() { RemoveAllIOWins(); }

CIOWinManager::IOWinPQNode::IOWinPQNode(rstl::ncrc_ptr< CIOWin > iowin, int prio,
                                        CIOWinManager::IOWinPQNode* next)
: mIowin(iowin), mPrio(prio), mNext(next) {}

void CIOWinManager::AddIOWin(rstl::ncrc_ptr< CIOWin > chIow, int pumpPrio, int drawPrio) {
  IOWinPQNode* prevNode = nullptr;
  IOWinPQNode* node;
  for (node = mPumpRoot; node != nullptr && node->GetPriority() > pumpPrio;
       node = node->GetNext()) {
    prevNode = node;
  }
  IOWinPQNode* newNode = rs_new IOWinPQNode(chIow, pumpPrio, node);
  if (!prevNode) {
    mPumpRoot = newNode;
  } else {
    prevNode->SetNext(newNode);
  }

  IOWinPQNode* prevDrawNode = nullptr;
  IOWinPQNode* drawNode;
  for (drawNode = mDrawRoot; drawNode != nullptr && drawNode->GetPriority() > drawPrio;
       drawNode = drawNode->GetNext()) {
    prevDrawNode = drawNode;
  }
  IOWinPQNode* newDrawNode = rs_new IOWinPQNode(chIow, drawPrio, drawNode);
  if (!prevDrawNode) {
    mDrawRoot = newDrawNode;
  } else {
    prevDrawNode->SetNext(newDrawNode);
  }
}

void CIOWinManager::RemoveIOWin(rstl::ncrc_ptr< CIOWin > chIow) {
  IOWinPQNode* node = mPumpRoot;
  IOWinPQNode* prevNode = nullptr;
  for (; node; node = node->GetNext()) {
    if (node->GetIOWin() == chIow) {
      if (prevNode == nullptr)
        mPumpRoot = node->GetNext();
      else
        prevNode->SetNext(node->GetNext());
      delete node;
      break;
    }
    prevNode = node;
  }

  prevNode = nullptr;
  for (IOWinPQNode* node = mDrawRoot; node; node = node->GetNext()) {
    if (node->GetIOWin() == chIow) {
      if (prevNode == nullptr)
        mDrawRoot = node->GetNext();
      else
        prevNode->SetNext(node->GetNext());
      delete node;
      break;
    }
    prevNode = node;
  }
}

void CIOWinManager::RemoveAllIOWins() {
  while (mDrawRoot) {
    RemoveIOWin(mDrawRoot->GetIOWin());
  }
  while (mPumpRoot) {
    RemoveIOWin(mPumpRoot->GetIOWin());
  }
}

void CIOWinManager::ChangeIOWinPriority(rstl::ncrc_ptr< CIOWin > toChange, int pumpPrio,
                                        int drawPrio) {
  IOWinPQNode* prevNode = nullptr;

  for (IOWinPQNode* node = mPumpRoot; node; node = node->GetNext()) {
    if (node->GetIOWin() == toChange) {
      if (prevNode == nullptr)
        mPumpRoot = node->GetNext();
      else
        prevNode->SetNext(node->GetNext());

      node->SetPriority(pumpPrio);
      IOWinPQNode* testPrevNode = nullptr;
      IOWinPQNode* testNode = mPumpRoot;
      for (; testNode && testNode->GetPriority() > pumpPrio; testNode = testNode->GetNext()) {
        testPrevNode = testNode;
      }
      node->SetNext(testNode);

      if (testPrevNode == nullptr)
        mPumpRoot = node;
      else
        testPrevNode->SetNext(node);
      break;
    }
    prevNode = node;
  }

  prevNode = nullptr;
  for (IOWinPQNode* node = mDrawRoot; node; node = node->GetNext()) {
    if (node->GetIOWin() == toChange) {
      if (prevNode == nullptr)
        mDrawRoot = node->GetNext();
      else
        prevNode->SetNext(node->GetNext());

      node->SetPriority(drawPrio);
      IOWinPQNode* testPrevNode = nullptr;
      IOWinPQNode* testNode = mDrawRoot;
      for (; testNode && testNode->GetPriority() > drawPrio; testNode = testNode->GetNext()) {
        testPrevNode = testNode;
      }
      node->SetNext(testNode);
      if (testPrevNode == nullptr)
        mDrawRoot = node;
      else
        testPrevNode->SetNext(node);
      break;
    }
    prevNode = node;
  }
}

rstl::ncrc_ptr< CIOWin > CIOWinManager::FindIOWin(const rstl::string& name) const {
  for (IOWinPQNode* node = mPumpRoot; node; node = node->GetNext()) {
    if (node->GetIOWin()->GetName() == name) {
      return node->GetIOWin();
    }
  }
  for (IOWinPQNode* node = mDrawRoot; node; node = node->GetNext()) {
    if (node->GetIOWin()->GetName() == name) {
      return node->GetIOWin();
    }
  }
  return rstl::ncrc_ptr< CIOWin >();
}

void CIOWinManager::PumpMessages(CArchitectureQueue& queue) {
  while (!queue.IsEmpty()) {
    CArchitectureMessage msg = queue.Pop();
    DistributeOneMessage(msg, queue);
  }
}

bool CIOWinManager::DistributeOneMessage(const CArchitectureMessage& msg,
                                         CArchitectureQueue& queue) {
  for (IOWinPQNode* node = mPumpRoot; node;) {
    rstl::ncrc_ptr< CIOWin > iow = node->GetIOWin();
    CIOWin::EMessageReturn mret = iow->OnMessage(msg, mLocalGatherQueue);

    while (!mLocalGatherQueue.IsEmpty()) {
      CArchitectureMessage msg = mLocalGatherQueue.Pop();
      if (msg.GetTarget() == kAMT_IOWinManager) {
        if (OnIOWinMessage(msg)) {
          mLocalGatherQueue.Clear();
          queue.Clear();
          return true;
        }
      } else {
        queue.Push(msg);
      }
    }

    if (mret == CIOWin::kMR_RemoveIOWinAndExit || mret == CIOWin::kMR_RemoveIOWin) {
      node = node->GetNext();
      RemoveIOWin(iow);
    } else {
      node = node->GetNext();
    }
    if (mret == CIOWin::kMR_Exit || mret == CIOWin::kMR_RemoveIOWinAndExit) {
      break;
    }
  }

  return false;
}

void CIOWinManager::Draw() const {
  for (IOWinPQNode* node = mDrawRoot; node; node = node->GetNext()) {
    node->GetIOWin()->PreDraw();
    if (!node->GetIOWin()->GetIsContinueDraw())
      break;
  }
  for (IOWinPQNode* node = mDrawRoot; node; node = node->GetNext()) {
    node->GetIOWin()->Draw();
    if (!node->GetIOWin()->GetIsContinueDraw())
      break;
  }
}

bool CIOWinManager::OnIOWinMessage(const CArchitectureMessage& msg) {
  switch (msg.GetType()) {
  case kAM_RemoveIOWin: {
    const CArchMsgParmString& parm = MakeMsg::GetParmDeleteIOWin(msg);
    rstl::ncrc_ptr< CIOWin > iow = FindIOWin(parm.GetString());
    if (iow)
      RemoveIOWin(iow);
    break;
  }
  case kAM_CreateIOWin: {
    const CArchMsgParmInt32Int32VoidPtr& parm = MakeMsg::GetParmCreateIOWin(msg);
    int pumpPrio = parm.GetFirstInt32();
    int drawPrio = parm.GetSecondInt32();
    const void* iow = parm.GetVoidPtr();
    rstl::ncrc_ptr< CIOWin > ptr(static_cast< CIOWin* >(const_cast< void* >(iow)));
    AddIOWin(ptr, pumpPrio, drawPrio);
    break;
  }
  case kAM_ChangeIOWinPriority: {
    const CArchMsgParmInt32Int32String& parm = MakeMsg::GetParmChangeIOWinPriority(msg);
    int pumpPrio = parm.GetFirstInt32();
    int drawPrio = parm.GetSecondInt32();
    rstl::ncrc_ptr< CIOWin > iow = FindIOWin(parm.GetString());
    if (iow)
      ChangeIOWinPriority(iow, pumpPrio, drawPrio);
    break;
  }
  case kAM_RemoveAllIOWins: {
    RemoveAllIOWins();
    return true;
  }
  default:
    break;
  }
  return false;
}

rstl::ncrc_ptr< CIOWin > CIOWinManager::IOWinPQNode::GetIOWin() const { return mIowin; }
