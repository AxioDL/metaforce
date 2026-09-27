#pragma once

class CIOWinManager;
int game_main(int argc, char* argv[]);
namespace metaforce {
int Initialize(int argc, char** argv);
void Shutdown();
int GetExitCode();
bool HasStartupRequest();
bool BeginFrame();
void EndFrame();
void RegisterIOWins(CIOWinManager& ioWinManager);
} // namespace metaforce
