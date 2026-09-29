#pragma once

class CIOWinManager;
namespace metaforce {
int Initialize(int argc, char** argv);
void Shutdown();
int GetExitCode();
bool HasStartupRequest();
bool BeginFrame();
void EndFrame();
void RegisterIOWins(CIOWinManager& ioWinManager);
} // namespace metaforce
