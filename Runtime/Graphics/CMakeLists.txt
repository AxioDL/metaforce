if(WIN32)
  set(PLAT_SRCS CLineRendererShadersHLSL.cpp)
elseif(APPLE)
  set(PLAT_SRCS CLineRendererShadersMetal.cpp)
endif()

add_library(RuntimeCommonGraphics
            IRenderer.hpp
            CBooRenderer.hpp CBooRenderer.cpp
            CLineRenderer.hpp CLineRenderer.cpp
            CLineRendererShaders.hpp CLineRendererShadersGLSL.cpp
            CMetroidModelInstance.hpp
            CLight.hpp CLight.cpp
            CTexture.hpp CTextureBoo.cpp
            CModel.hpp CModelBoo.cpp
            CGraphics.hpp CGraphics.cpp
            ${PLAT_SRCS})
