list(APPEND PY_SOURCES
     hecl/__init__.py
     hecl/Nodegrid.py
     hecl/hmdl/__init__.py
     hecl/hmdl/HMDLMesh.py
     hecl/hmdl/HMDLShader.py
     hecl/hmdl/HMDLSkin.py
     hecl/hmdl/HMDLTxtr.py
     hecl/sact/__init__.py
     hecl/sact/SACTAction.py
     hecl/sact/SACTEvent.py
     hecl/sact/SACTSubtype.py)

bintoc(hecl_blendershell.c hecl_blendershell.py HECL_BLENDERSHELL)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/hecl.zip)
message("-- Generating addon package")
execute_process(COMMAND python zip_package.py
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/hecl.zip)
message(FATAL_ERROR "Unable to generate ${CMAKE_CURRENT_SOURCE_DIR}/hecl.zip; is python installed?")
endif()
endif()
bintoc(hecl_addon.c hecl.zip HECL_ADDON)

add_library(HECLBlender
            BlenderConnection.cpp
            BlenderConnection.hpp
            hecl_blendershell.py
            hecl_blendershell.c
            hecl_addon.c
            ${PY_SOURCES})
