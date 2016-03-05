#include <stdint.h>
#include <BlenderConnection.hpp>
#include "BlenderSupport.hpp"

extern "C" uint8_t RETRO_MASTER_SHADER[];
extern "C" size_t RETRO_MASTER_SHADER_SZ;

namespace DataSpec
{
namespace Blender
{

bool BuildMasterShader(const hecl::ProjectPath& path)
{
    hecl::BlenderConnection& conn = hecl::BlenderConnection::SharedConnection();
    if (!conn.createBlend(path, hecl::BlenderConnection::BlendType::None))
        return false;
    {
        hecl::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
        os << RETRO_MASTER_SHADER;
        os << "make_master_shader_library()\n";
    }
    return conn.saveBlend();
}

}
}
