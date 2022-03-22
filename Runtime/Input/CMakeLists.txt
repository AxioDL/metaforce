set(INPUT_SOURCES
        PAD.hpp
        IController.hpp DolphinIController.cpp
        CControllerAxis.hpp
        CControllerButton.hpp
        CControllerGamepadData.hpp CControllerGamepadData.cpp
        CDolphinController.hpp CDolphinController.cpp
        CKeyboardMouseController.hpp
        ControlMapper.hpp ControlMapper.cpp
        CInputGenerator.hpp CInputGenerator.cpp
        CFinalInput.hpp CFinalInput.cpp
        CRumbleManager.hpp CRumbleManager.cpp
        CRumbleGenerator.hpp CRumbleGenerator.cpp
        CRumbleVoice.hpp CRumbleVoice.cpp
        RumbleFxTable.hpp RumbleFxTable.cpp)

runtime_add_list(Input INPUT_SOURCES)
