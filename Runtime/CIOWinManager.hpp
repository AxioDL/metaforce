#pragma once

#include <memory>
#include <list>
#include "CIOWin.hpp"
#include "rstl.hpp"
#include "CArchitectureQueue.hpp"

namespace urde {

class CIOWinManager {
  struct IOWinPQNode {
    std::shared_ptr<CIOWin> x0_iowin;
    int x4_prio;
    CIOWinManager::IOWinPQNode* x8_next = nullptr;
    IOWinPQNode(std::weak_ptr<CIOWin> iowin, int prio, CIOWinManager::IOWinPQNode* next)
    : x0_iowin(iowin), x4_prio(prio), x8_next(next) {}
    std::shared_ptr<CIOWin> ShareIOWin() const { return std::shared_ptr<CIOWin>(x0_iowin); }
    CIOWin* GetIOWin() const { return x0_iowin.get(); }
  };
  IOWinPQNode* x0_drawRoot = nullptr;
  IOWinPQNode* x4_pumpRoot = nullptr;
  CArchitectureQueue x8_localGatherQueue;

public:
  bool OnIOWinMessage(const CArchitectureMessage& msg);
  void Draw() const;
  bool DistributeOneMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
  void PumpMessages(CArchitectureQueue& queue);
  CIOWin* FindIOWin(std::string_view name);
  std::shared_ptr<CIOWin> FindAndShareIOWin(std::string_view name);
  void ChangeIOWinPriority(CIOWin* toChange, int pumpPrio, int drawPrio);
  void RemoveAllIOWins();
  void RemoveIOWin(CIOWin* toRemove);
  void AddIOWin(std::weak_ptr<CIOWin> toAdd, int pumpPrio, int drawPrio);
  bool IsEmpty() const { return x0_drawRoot == nullptr && x4_pumpRoot == nullptr; }
};

} // namespace urde
