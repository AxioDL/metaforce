#include <stdint.h>
#include <BlenderConnection.hpp>
#include "BlenderSupport.hpp"

extern "C" uint8_t RETRO_MASTER_SHADER[];
extern "C" size_t RETRO_MASTER_SHADER_SZ;

namespace Retro
{
namespace Blender
{

bool BuildMasterShader(const HECL::ProjectPath& path)
{
    HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
    if (!conn.createBlend(path, HECL::BlenderConnection::BlendType::None))
        return false;
    {
        HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
        os << RETRO_MASTER_SHADER;
        os << "make_master_shader_library()\n";
    }
    return conn.saveBlend();
}

}
}
