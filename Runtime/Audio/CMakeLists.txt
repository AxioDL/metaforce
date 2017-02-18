set(AUDIO_SOURCES
    CAudioSys.hpp CAudioSys.cpp
    CAudioGroupSet.hpp CAudioGroupSet.cpp
    CSfxManager.hpp CSfxManager.cpp
    CMidiManager.hpp CMidiManager.cpp
    CStaticAudioPlayer.hpp CStaticAudioPlayer.cpp
    CStreamAudioManager.hpp CStreamAudioManager.cpp
    g721.c g721.h)

runtime_add_list(Audio AUDIO_SOURCES)
