list(APPEND PY_SOURCES
     addon/__init__.py
     addon/hmdl/__init__.py
     addon/hmdl/HMDLMesh.py
     addon/hmdl/HMDLShader.py
     addon/hmdl/HMDLSkin.py
     addon/hmdl/HMDLTxtr.py
     addon/sact/__init__.py
     addon/sact/SACTAction.py
     addon/sact/SACTEvent.py
     addon/sact/SACTSubtype.py)

add_library(HECLBlender
            BlenderConnection.cpp
            BlenderConnection.hpp
            blendershell.py
            ${PY_SOURCES})
