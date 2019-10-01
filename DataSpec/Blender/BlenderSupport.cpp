#include <cstdint>
#include "hecl/Blender/Connection.hpp"
#include "BlenderSupport.hpp"

extern "C" uint8_t RETRO_MASTER_SHADER[];
extern "C" size_t RETRO_MASTER_SHADER_SZ;

namespace DataSpec::Blender {

bool BuildMasterShader(const hecl::ProjectPath& path) {
  hecl::blender::Connection& conn = hecl::blender::Connection::SharedConnection();
  if (!conn.createBlend(path, hecl::blender::BlendType::None))
    return false;
  {
    hecl::blender::PyOutStream os = conn.beginPythonOut(true);
    os << std::string_view((char*)RETRO_MASTER_SHADER, RETRO_MASTER_SHADER_SZ);
    os << "make_master_shader_library()\n"sv;
  }
  return conn.saveBlend();
}

} // namespace DataSpec::Blender
