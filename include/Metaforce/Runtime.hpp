#pragma once

class CIOWinManager;
namespace metaforce {
int Initialize(int argc, char** argv);
void Shutdown();
int GetExitCode();
void RequestQuit();
bool HasStartupRequest();
bool BeginFrame();
void EndFrame();
void RegisterIOWins(CIOWinManager& ioWinManager);
} // namespace metaforce
