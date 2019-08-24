#pragma once

namespace hecl::blender {
struct Light;
class PyOutStream;
}

namespace DataSpec {

template <class BabeDeadLight>
void ReadBabeDeadLightToBlender(hecl::blender::PyOutStream& os, const BabeDeadLight& light, unsigned s, unsigned l);

template <class BabeDeadLight>
void WriteBabeDeadLightFromBlender(BabeDeadLight& lightOut, const hecl::blender::Light& lightIn);

} // namespace DataSpec
