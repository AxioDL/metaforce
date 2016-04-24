set(INPUT_SOURCES
    IController.hpp
    CKeyboardMouseController.hpp
    ControlMapper.hpp ControlMapper.cpp
    CInputGenerator.hpp CInputGenerator.cpp
    CFinalInput.hpp CFinalInput.cpp
    CRumbleManager.hpp CRumbleManager.cpp
    CRumbleGenerator.hpp CRumbleGenerator.cpp
    CRumbleVoice.hpp CRumbleVoice.cpp)

runtime_add_list(Input INPUT_SOURCES)
