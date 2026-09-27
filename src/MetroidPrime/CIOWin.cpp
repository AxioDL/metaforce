#include "MetroidPrime/CIOWin.hpp"

CIOWin::CIOWin(const rstl::string& name) : mName(name) {}

CIOWin::~CIOWin() {}

const rstl::string& CIOWin::GetName() const { return mName; }

bool CIOWin::GetIsContinueDraw() const { return true; }

void CIOWin::Draw() const {}

void CIOWin::PreDraw() const {}
