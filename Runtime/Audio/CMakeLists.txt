set(AUDIO_SOURCES
    CAudioSys.hpp CAudioSys.cpp
    CAudioStateWin.hpp CAudioStateWin.cpp
    CSfxManager.hpp CSfxManager.cpp
    CSfxHandle.hpp CSfxHandle.cpp
    dsp.c dsp.h
    g721.c g721.h)

runtime_add_list(Audio AUDIO_SOURCES)
