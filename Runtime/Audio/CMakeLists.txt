set(AUDIO_SOURCES
    CAudioSys.hpp CAudioSys.cpp
    CAudioStateWin.hpp CAudioStateWin.cpp
    CAudioGroupSet.hpp CAudioGroupSet.cpp
    CSfxManager.hpp CSfxManager.cpp
    CSfxHandle.hpp CSfxHandle.cpp
    g721.c g721.h)

runtime_add_list(Audio AUDIO_SOURCES)
