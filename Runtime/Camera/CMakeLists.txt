set(CAMERA_SOURCES
    CCameraManager.hpp CCameraManager.cpp
    CGameCamera.hpp CGameCamera.cpp
    CFirstPersonCamera.hpp CFirstPersonCamera.cpp
    CBallCamera.hpp CBallCamera.cpp
    CInterpolationCamera.hpp CInterpolationCamera.cpp
    CPathCamera.hpp CPathCamera.cpp
    CCinematicCamera.hpp CCinematicCamera.cpp
    CCameraShakeData.hpp CCameraShakeData.cpp
    CCameraFilter.hpp CCameraFilter.cpp
    CCameraSpline.hpp CCameraSpline.cpp)

runtime_add_list(Camera CAMERA_SOURCES)
