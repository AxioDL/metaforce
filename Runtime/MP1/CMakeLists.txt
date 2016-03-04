include_directories(. ..)
add_library(RuntimeMP1
            CTweaks.hpp CTweaks.cpp
            CInGameTweakManager.hpp CInGameTweakManager.cpp
            CMainFlow.hpp CMainFlow.cpp
            CMFGame.hpp CMFGame.cpp
            CPlayMovie.hpp CPlayMovie.cpp
            CFrontEndUI.hpp CFrontEndUI.cpp
            MP1.hpp MP1.cpp)
