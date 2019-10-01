#include "MeshOptimizer.hpp"

#include <algorithm>
#include <cfloat>
#include <climits>
#include <cmath>
#include <numeric>
#include <unordered_set>

namespace hecl::blender {

logvisor::Module Log("MeshOptimizer");

template <typename T>
static void insert_unique_attr(std::unordered_map<T, uint32_t>& set, const T& attr) {
  if (set.find(attr) == set.cend()) {
    size_t sz = set.size();
    set.insert(std::make_pair(attr, sz));
  }
}

template <typename T>
static std::vector<T> sort_unordered_map(const std::unordered_map<T, uint32_t>& map) {
  struct SortableIterator {
    typename std::unordered_map<T, uint32_t>::const_iterator it;
    bool operator<(const SortableIterator& other) const { return it->second < other.it->second; }
    explicit SortableIterator(typename std::unordered_map<T, uint32_t>::const_iterator i) : it(i) {}
  };
  std::vector<SortableIterator> to_sort;
  to_sort.reserve(map.size());
  for (auto I = map.cbegin(), E = map.cend(); I != E; ++I)
    to_sort.emplace_back(I);
  std::sort(to_sort.begin(), to_sort.end());
  std::vector<T> ret;
  ret.reserve(to_sort.size());
  for (const auto& sit : to_sort)
    ret.push_back(sit.it->first);
  return ret;
}

static bool material_is_lightmapped(const Material& mat) {
  auto search = mat.iprops.find("retro_lightmapped");
  if (search != mat.iprops.cend())
    return search->second;
  return false;
}

MeshOptimizer::Vertex::Vertex(Connection& conn) {
  co.read(conn);
  uint32_t skin_count;
  conn._readValue(skin_count);
  if (skin_count > MaxSkinEntries)
    Log.report(logvisor::Fatal, fmt("Skin entry overflow {}/{}"), skin_count, MaxSkinEntries);
  for (uint32_t i = 0; i < skin_count; ++i)
    skin_ents[i] = Mesh::SkinBind(conn);
}

MeshOptimizer::Loop::Loop(Connection& conn, uint32_t color_count, uint32_t uv_count) {
  normal.read(conn);
  for (uint32_t i = 0; i < color_count; ++i)
    colors[i].read(conn);
  for (uint32_t i = 0; i < uv_count; ++i)
    uvs[i].read(conn);
  conn._readValue(vert);
  conn._readValue(edge);
  conn._readValue(face);
  conn._readValue(link_loop_next);
  conn._readValue(link_loop_prev);
  conn._readValue(link_loop_radial_next);
  conn._readValue(link_loop_radial_prev);
}

MeshOptimizer::Edge::Edge(Connection& conn) {
  for (uint32_t i = 0; i < 2; ++i)
    conn._readValue(verts[i]);
  uint32_t face_count;
  conn._readValue(face_count);
  if (face_count > MaxLinkFaces)
    Log.report(logvisor::Fatal, fmt("Face overflow {}/{}"), face_count, MaxLinkFaces);
  for (uint32_t i = 0; i < face_count; ++i)
    conn._readValue(link_faces[i]);
  conn._readValue(is_contiguous);
}

MeshOptimizer::Face::Face(Connection& conn) {
  normal.read(conn);
  centroid.read(conn);
  conn._readValue(material_index);
  for (uint32_t i = 0; i < 3; ++i)
    conn._readValue(loops[i]);
}

uint32_t MeshOptimizer::get_pos_idx(const Vertex& v) const {
  auto search = b_pos.find(v.co);
  if (search != b_pos.cend())
    return search->second;
  return UINT32_MAX;
}

uint32_t MeshOptimizer::get_norm_idx(const Loop& l) const {
  auto search = b_norm.find(l.normal);
  if (search != b_norm.cend())
    return search->second;
  return UINT32_MAX;
}

uint32_t MeshOptimizer::get_skin_idx(const Vertex& v) const {
  auto search = b_skin.find(v.skin_ents);
  if (search != b_skin.cend())
    return search->second;
  return UINT32_MAX;
}

uint32_t MeshOptimizer::get_color_idx(const Loop& l, uint32_t cidx) const {
  auto search = b_color.find(l.colors[cidx]);
  if (search != b_color.cend())
    return search->second;
  return UINT32_MAX;
}

uint32_t MeshOptimizer::get_uv_idx(const Loop& l, uint32_t uidx) const {
  if (use_luvs && uidx == 0 && material_is_lightmapped(materials[faces[l.face].material_index])) {
    auto search = b_luv.find(l.uvs[0]);
    if (search != b_luv.cend())
      return search->second;
    return UINT32_MAX;
  }
  auto search = b_uv.find(l.uvs[uidx]);
  if (search != b_uv.cend())
    return search->second;
  return UINT32_MAX;
}

bool MeshOptimizer::loops_contiguous(const Loop& la, const Loop& lb) const {
  if (la.vert != lb.vert)
    return false;
  if (get_norm_idx(la) != get_norm_idx(lb))
    return false;
  for (uint32_t i = 0; i < color_count; ++i)
    if (get_color_idx(la, i) != get_color_idx(lb, i))
      return false;
  for (uint32_t i = 0; i < uv_count; ++i)
    if (get_uv_idx(la, i) != get_uv_idx(lb, i))
      return false;
  return true;
}

bool MeshOptimizer::splitable_edge(const Edge& e) const {
  if (!e.is_contiguous)
    return false;
  for (uint32_t vidx : e.verts) {
    const Loop* found = nullptr;
    for (uint32_t fidx : e.link_faces) {
      for (uint32_t lidx : faces[fidx].loops) {
        if (loops[lidx].vert == vidx) {
          if (!found) {
            found = &loops[lidx];
            break;
          } else {
            if (!loops_contiguous(*found, loops[lidx]))
              return true;
            break;
          }
        }
      }
    }
  }
  return false;
}

void MeshOptimizer::sort_faces_by_skin_group(std::vector<uint32_t>& sfaces) const {
  std::vector<uint32_t> faces_out;
  faces_out.reserve(sfaces.size());
  std::unordered_set<uint32_t> done_sg;
  uint32_t ref_sg = UINT32_MAX;
  while (faces_out.size() < sfaces.size()) {
    for (uint32_t f : sfaces) {
      bool found = false;
      for (uint32_t l : faces[f].loops) {
        uint32_t skin_idx = get_skin_idx(verts[loops[l].vert]);
        if (done_sg.find(skin_idx) == done_sg.end()) {
          ref_sg = skin_idx;
          done_sg.insert(skin_idx);
          found = true;
          break;
        }
      }
      if (found)
        break;
    }
    for (uint32_t f : sfaces) {
      if (std::find(faces_out.begin(), faces_out.end(), f) != faces_out.end())
        continue;
      for (uint32_t l : faces[f].loops) {
        uint32_t skin_idx = get_skin_idx(verts[loops[l].vert]);
        if (skin_idx == ref_sg) {
          faces_out.push_back(f);
          break;
        }
      }
    }
  }
  sfaces = std::move(faces_out);
}

std::pair<uint32_t, uint32_t> MeshOptimizer::strip_next_loop(uint32_t prev_loop, uint32_t out_count) const {
  if (out_count & 0x1) {
    uint32_t radial_loop = loops[prev_loop].link_loop_radial_next;
    uint32_t loop = loops[radial_loop].link_loop_prev;
    return {loop, loop};
  } else {
    uint32_t radial_loop = loops[prev_loop].link_loop_radial_prev;
    uint32_t loop = loops[radial_loop].link_loop_next;
    return {loops[loop].link_loop_next, loop};
  }
}

static float Magnitude(const Vector3f& v) { return std::sqrt(v.val.simd.dot3(v.val.simd)); }
static void Normalize(Vector3f& v) {
  float mag = 1.f / Magnitude(v);
  v.val.simd *= athena::simd<float>(mag);
}

Mesh::Surface MeshOptimizer::generate_surface(std::vector<uint32_t>& island_faces, uint32_t mat_idx) const {
  Mesh::Surface ret = {};
  ret.materialIdx = mat_idx;

  /* Centroid of surface */
  for (const auto& f : island_faces)
    ret.centroid.val.simd += faces[f].centroid.val.simd;
  ret.centroid.val.simd /= athena::simd<float>(island_faces.size());

  /* AABB of surface */
  ret.aabbMin.val.simd = athena::simd<float>(FLT_MAX);
  ret.aabbMax.val.simd = athena::simd<float>(-FLT_MAX);
  for (const auto& f : island_faces) {
    for (const auto& l : faces[f].loops) {
      const Vertex& v = verts[loops[l].vert];
      for (int c = 0; c < 3; ++c) {
        if (v.co.val.simd[c] < ret.aabbMin.val.simd[c])
          ret.aabbMin.val.simd[c] = v.co.val.simd[c];
        if (v.co.val.simd[c] > ret.aabbMax.val.simd[c])
          ret.aabbMax.val.simd[c] = v.co.val.simd[c];
      }
    }
  }

  /* Average normal of surface */
  for (const auto& f : island_faces)
    ret.reflectionNormal.val.simd += faces[f].normal.val.simd;
  Normalize(ret.reflectionNormal);

  /* Verts themselves */
  uint32_t prev_loop_emit = UINT32_MAX;
  std::vector<std::pair<std::vector<uint32_t>, std::vector<uint32_t>>> sel_lists_local;
  sel_lists_local.reserve(loops.size());
  while (island_faces.size()) {
    sel_lists_local.clear();
    for (uint32_t start_face : island_faces) {
      for (uint32_t l : faces[start_face].loops) {
        std::vector<uint32_t> island_local(island_faces);
        uint32_t prev_loop = loops[l].link_loop_next;
        uint32_t loop = loops[prev_loop].link_loop_next;
        std::vector<uint32_t> sel_list;
        sel_list.reserve(64);
        sel_list.push_back(l);
        sel_list.push_back(prev_loop);
        sel_list.push_back(loop);
        island_local.erase(std::find(island_local.begin(), island_local.end(), start_face));
        while (true) {
          const Edge& prev_edge = edges[loops[prev_loop].edge];
          if (!prev_edge.is_contiguous || prev_edge.tag)
            break;
          std::tie(loop, prev_loop) = strip_next_loop(prev_loop, sel_list.size());
          uint32_t face = loops[loop].face;
          auto search = std::find(island_local.begin(), island_local.end(), face);
          if (search == island_local.end())
            break;
          sel_list.push_back(loop);
          island_local.erase(search);
        }
        sel_lists_local.emplace_back(std::move(sel_list), std::move(island_local));
      }
    }
    uint32_t max_count = 0;
    const std::vector<uint32_t>* max_sl = nullptr;
    const std::vector<uint32_t>* max_island_faces = nullptr;
    for (const auto& sl : sel_lists_local) {
      if (sl.first.size() > max_count) {
        max_count = sl.first.size();
        max_sl = &sl.first;
        max_island_faces = &sl.second;
      }
    }
    assert(max_island_faces && "Should not be null");
    assert(max_island_faces->size() < island_faces.size() && "Infinite loop condition");
    island_faces = std::move(*max_island_faces);
    if (prev_loop_emit != UINT32_MAX)
      ret.verts.emplace_back();
    for (uint32_t loop : *max_sl) {
      ret.verts.emplace_back();
      const auto& l = loops[loop];
      auto& vert = ret.verts.back();
      vert.iPos = get_pos_idx(verts[l.vert]);
      vert.iNorm = get_norm_idx(l);
      for (uint32_t i = 0; i < color_count; ++i)
        vert.iColor[i] = get_color_idx(l, i);
      for (uint32_t i = 0; i < uv_count; ++i)
        vert.iUv[i] = get_uv_idx(l, i);
      vert.iSkin = get_skin_idx(verts[l.vert]);
      prev_loop_emit = loop;
    }
  }

  return ret;
}

void MeshOptimizer::optimize(Mesh& mesh, int max_skin_banks) const {
  mesh.topology = HMDLTopology::TriStrips;

  mesh.pos = sort_unordered_map(b_pos);
  mesh.norm = sort_unordered_map(b_norm);
  mesh.colorLayerCount = color_count;
  mesh.color = sort_unordered_map(b_color);
  mesh.uvLayerCount = uv_count;
  mesh.uv = sort_unordered_map(b_uv);
  mesh.luv = sort_unordered_map(b_luv);
  mesh.skins = sort_unordered_map(b_skin);

  /* Sort materials by pass index */
  std::vector<uint32_t> sorted_material_idxs(materials.size());
  std::iota(sorted_material_idxs.begin(), sorted_material_idxs.end(), 0);
  std::sort(sorted_material_idxs.begin(), sorted_material_idxs.end(),
  [this](uint32_t a, uint32_t b) { return materials[a].passIndex < materials[b].passIndex; });

  /* Generate island surfaces */
  std::vector<uint32_t> mat_faces_rem, the_list;
  mat_faces_rem.reserve(faces.size());
  the_list.reserve(faces.size());
  std::unordered_set<uint32_t> skin_slot_set;
  skin_slot_set.reserve(b_skin.size());
  for (uint32_t mat_idx : sorted_material_idxs) {
    const auto& mat = materials[mat_idx];
    mat_faces_rem.clear();
    for (auto B = faces.begin(), I = B, E = faces.end(); I != E; ++I) {
      if (I->material_index == mat_idx)
        mat_faces_rem.push_back(I - B);
    }
    if (b_skin.size())
      sort_faces_by_skin_group(mat_faces_rem);
    size_t rem_count = mat_faces_rem.size();
    while (rem_count) {
      the_list.clear();
      skin_slot_set.clear();
      for (uint32_t& f : mat_faces_rem) {
        if (f == UINT32_MAX)
          continue;
        if (b_skin.size()) {
          bool brk = false;
          for (const auto& l : faces[f].loops) {
            const Vertex& v = verts[loops[l].vert];
            uint32_t skin_idx = get_skin_idx(v);
            if (skin_slot_set.find(skin_idx) == skin_slot_set.end()) {
              if (max_skin_banks > 0 && skin_slot_set.size() == size_t(max_skin_banks)) {
                brk = true;
                break;
              }
              skin_slot_set.insert(skin_idx);
            }
          }
          if (brk)
            break;
        }
        the_list.push_back(f);
        f = UINT32_MAX;
        --rem_count;
      }
      mesh.surfaces.push_back(generate_surface(the_list, mat_idx));
    }
  }
}

MeshOptimizer::MeshOptimizer(Connection& conn, const std::vector<Material>& materials, bool use_luvs)
: materials(materials), use_luvs(use_luvs) {
  conn._readValue(color_count);
  if (color_count > MaxColorLayers)
    Log.report(logvisor::Fatal, fmt("Color layer overflow {}/{}"), color_count, MaxColorLayers);
  conn._readValue(uv_count);
  if (uv_count > MaxUVLayers)
    Log.report(logvisor::Fatal, fmt("UV layer overflow {}/{}"), uv_count, MaxUVLayers);

  /* Simultaneously load topology objects and build unique mapping indices */

  uint32_t vert_count;
  conn._readValue(vert_count);
  verts.reserve(vert_count);
  b_pos.reserve(vert_count);
  b_skin.reserve(vert_count * 4);
  for (uint32_t i = 0; i < vert_count; ++i) {
    verts.emplace_back(conn);
    insert_unique_attr(b_pos, verts.back().co);
    if (verts.back().skin_ents[0].valid())
      insert_unique_attr(b_skin, verts.back().skin_ents);
  }

  uint32_t loop_count;
  conn._readValue(loop_count);
  loops.reserve(loop_count);
  b_norm.reserve(loop_count);
  if (use_luvs) {
    b_uv.reserve(std::max(int(loop_count) - 1, 0) * uv_count);
    b_luv.reserve(loop_count);
  } else {
    b_uv.reserve(loop_count * uv_count);
  }
  for (uint32_t i = 0; i < loop_count; ++i) {
    loops.emplace_back(conn, color_count, uv_count);
    insert_unique_attr(b_norm, loops.back().normal);
    for (const auto& c : loops.back().colors)
      insert_unique_attr(b_color, c);
    if (use_luvs && material_is_lightmapped(materials[faces[loops.back().face].material_index])) {
      insert_unique_attr(b_luv, loops.back().uvs[0]);
      for (auto I = std::begin(loops.back().uvs) + 1, E = std::end(loops.back().uvs); I != E; ++I)
        insert_unique_attr(b_uv, *I);
    } else {
      for (const auto& c : loops.back().uvs)
        insert_unique_attr(b_uv, c);
    }
  }

  conn._readVector(edges);
  conn._readVector(faces);

  /* Cache edges that should block tristrip traversal */
  for (auto& e : edges)
    e.tag = splitable_edge(e);
}

}
