#include "shader.hpp"

#include "../../gpu.hpp"
#include "../common.hpp"

#include <aurora/model.hpp>
#include <magic_enum.hpp>

enum class VtxDescAttr : u8 {
  Position = 0,
  Normal = 2,
  Color0 = 4,
  Color1 = 8,
  Tex0 = 10,
  Tex1 = 12,
  Tex2 = 14,
  Tex3 = 16,
  Tex4 = 18,
  Tex5 = 20,
  Tex6 = 22,
  PnMatIdx = 24,
  Tex0MatIdx = 25,
  Tex1MatIdx = 26,
  Tex2MatIdx = 27,
  Tex3MatIdx = 28,
  Tex4MatIdx = 29,
  Tex5MatIdx = 30,
  Tex6MatIdx = 31,
};
enum class VtxDescAttrType : u8 { None = 0, Direct = 1, Index8 = 2, Index16 = 3 };
class VtxDescFlags {
  u32 m_flags = 0;

public:
  constexpr VtxDescFlags() noexcept = default;
  constexpr VtxDescFlags(u32 flags) noexcept : m_flags(flags){};
  [[nodiscard]] constexpr VtxDescAttrType GetAttributeType(VtxDescAttr attribute) const noexcept {
    return VtxDescAttrType((m_flags >> u32(attribute)) & 0x3);
  }
  [[nodiscard]] constexpr VtxDescAttrType GetDirectAttributeType(VtxDescAttr attribute) const noexcept {
    return VtxDescAttrType((m_flags >> u32(attribute)) & 0x1);
  }
  constexpr void SetAttributeType(VtxDescAttr attribute, VtxDescAttrType type) noexcept {
    m_flags &= ~(u32(0x3) << u32(attribute));
    m_flags |= u32(type) << u32(attribute);
  }
};

namespace aurora::gfx::model {
static logvisor::Module Log("aurora::gfx::model");

static const std::vector<zeus::CVector3f>* vtxData;
static const std::vector<zeus::CVector3f>* nrmData;
static const std::vector<zeus::CVector2f>* tex0TcData;
static const std::vector<zeus::CVector2f>* tcData;

void set_vertex_buffer(const std::vector<zeus::CVector3f>& data) noexcept { vtxData = &data; }
void set_normal_buffer(const std::vector<zeus::CVector3f>& norm) noexcept { nrmData = &norm; }
void set_tex0_tc_buffer(const std::vector<zeus::CVector2f>& tcs) noexcept { tex0TcData = &tcs; }
void set_tc_buffer(const std::vector<zeus::CVector2f>& tcs) noexcept { tcData = &tcs; }

struct DlVert {
  s16 pos;
  s16 norm;
  // colors ignored
  std::array<s16, 7> uvs;
  // pn_mtx_idx ignored
  // tex_mtx_idxs ignored
  s16 _pad;
};

enum class VertexFormat : u8 {
  F32F32,
  S16F32,
  S16S16,
};
static VtxDescFlags sVtxDescFlags;
void set_vtx_desc_compressed(u32 vtxDesc) noexcept { sVtxDescFlags = vtxDesc; }

static inline std::pair<DlVert, size_t> readVert(const u8* data) noexcept {
  DlVert out{};
  size_t offset = 0;
  const auto read8 = [data, &offset](VtxDescAttrType type) -> s8 {
    if (type == VtxDescAttrType::Direct) {
      s8 v = static_cast<s8>(data[offset]);
      ++offset;
      return v;
    }
    return 0;
  };
  const auto read16 = [data, &offset](VtxDescAttrType type) -> s16 {
    if (type == VtxDescAttrType::Index16) {
      s16 v = metaforce::SBig(*reinterpret_cast<const u16*>(data + offset));
      offset += 2;
      return v;
    }
    return 0;
  };
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::PnMatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex0MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex1MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex2MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex3MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex4MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex5MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex6MatIdx));

  out.pos = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Position));
  out.norm = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Normal));
  read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Color0));
  read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Color1));
  out.uvs[0] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex0));
  out.uvs[1] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex1));
  out.uvs[2] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex2));
  out.uvs[3] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex3));
  out.uvs[4] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex4));
  out.uvs[5] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex5));
  out.uvs[6] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex6));

  return {out, offset};
}

void queue_surface(const u8* dlStart, u32 dlSize) noexcept {
  //  Log.report(logvisor::Info, FMT_STRING("DL size {}"), dlSize);
  std::vector<DlVert> verts;
  std::vector<u32> indices;

  size_t offset = 0;
  while (offset < dlSize) {
    const auto header = dlStart[offset];
    const auto primitive = static_cast<GX::Primitive>(header & 0xF8);
    const auto vtxFmt = static_cast<VertexFormat>(header & 0x3);
    const auto vtxCount = metaforce::SBig(*reinterpret_cast<const u16*>(dlStart + offset + 1));
    //    Log.report(logvisor::Info, FMT_STRING("DL header prim {}, fmt {}, vtx count {}"), primitive,
    //               magic_enum::enum_name(vtxFmt), vtxCount);
    offset += 3;

    if (primitive == 0) {
      break;
    }
    if (primitive != GX::TRIANGLES && primitive != GX::TRIANGLESTRIP && primitive != GX::TRIANGLEFAN) {
      Log.report(logvisor::Fatal, FMT_STRING("queue_surface: unsupported primitive type {}"), primitive);
      unreachable();
    }

    const u32 idxStart = indices.size();
    const u16 vertsStart = verts.size();
    verts.reserve(vertsStart + vtxCount);
    if (vtxCount > 3 && (primitive == GX::TRIANGLEFAN || primitive == GX::TRIANGLESTRIP)) {
      indices.reserve(idxStart + (u32(vtxCount) - 3) * 3 + 3);
    } else {
      indices.reserve(idxStart + vtxCount);
    }
    auto curVert = vertsStart;
    for (int v = 0; v < vtxCount; ++v) {
      const auto [vert, read] = readVert(dlStart + offset);
      verts.push_back(vert);
      offset += read;
      if (primitive == GX::TRIANGLES || v < 3) {
        // pass
      } else if (primitive == GX::TRIANGLEFAN) {
        indices.push_back(vertsStart);
        indices.push_back(curVert - 1);
      } else if (primitive == GX::TRIANGLESTRIP) {
        if ((v & 1) == 0) {
          indices.push_back(curVert - 2);
          indices.push_back(curVert - 1);
        } else {
          indices.push_back(curVert - 1);
          indices.push_back(curVert - 2);
        }
      }
      indices.push_back(curVert);
      ++curVert;
    }
  }

  //  Log.report(logvisor::Info, FMT_STRING("Read {} verts, {} indices"), verts.size(), indices.size());
}
} // namespace aurora::gfx::model
