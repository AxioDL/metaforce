#pragma once

union SDL_Event;

namespace metaforce::ui {

bool Initialize();
void Shutdown();
void HandleEvent(const SDL_Event& event);
void Update();

} // namespace metaforce::ui
