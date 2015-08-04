list(APPEND PY_SOURCES
     addon/__init__.py
     addon/Nodegrid.py
     addon/hmdl/__init__.py
     addon/hmdl/HMDLMesh.py
     addon/hmdl/HMDLShader.py
     addon/hmdl/HMDLSkin.py
     addon/hmdl/HMDLTxtr.py
     addon/sact/__init__.py
     addon/sact/SACTAction.py
     addon/sact/SACTEvent.py
     addon/sact/SACTSubtype.py)

bintoc(blendershell.c blendershell.py BLENDERSHELL)

add_library(HECLBlender
            BlenderConnection.cpp
            BlenderConnection.hpp
            blendershell.py
            blendershell.c
            ${PY_SOURCES})
