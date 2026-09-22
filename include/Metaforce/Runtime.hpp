#pragma once

namespace metaforce {
int Initialize(int argc, char** argv);
void Shutdown();
bool BeginFrame();
void EndFrame();
} // namespace metaforce