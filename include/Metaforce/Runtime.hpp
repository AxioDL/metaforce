#pragma once

namespace metaforce {
int Initialize(int argc, char** argv);
void Shutdown();
int GetExitCode();
bool HasStartupRequest();
bool BeginFrame();
void EndFrame();
} // namespace metaforce
