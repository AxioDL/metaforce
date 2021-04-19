#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "hecl/Blender/Connection.hpp"

namespace hecl::blender {

template <size_t S>
class IndexArray {
  std::array<uint32_t, S> arr;
public:
  IndexArray() { std::fill(arr.begin(), arr.end(), UINT32_MAX); }
  class const_iterator {
    typename std::array<uint32_t, S>::const_iterator it;
  public:
    explicit const_iterator(typename std::array<uint32_t, S>::const_iterator i) : it(i) {}
    bool operator==(const_iterator other) const { return it == other.it; }
    bool operator!=(const_iterator other) const { return it != other.it; }
    const_iterator& operator++() { ++it; return *this; }
    uint32_t operator*() const { return *it; }
  };
  const_iterator begin() const { return const_iterator(arr.cbegin()); }
  const_iterator end() const {
    typename std::array<uint32_t, S>::const_iterator I, E;
    for (I = arr.cbegin(), E = arr.cend(); I != E && *I != UINT32_MAX; ++I) {}
    return const_iterator(I);
  }
  uint32_t& operator[](size_t idx) { return arr[idx]; }
  const uint32_t& operator[](size_t idx) const { return arr[idx]; }
  static constexpr size_t capacity() { return S; }
  size_t size() const { return end() - begin(); }
};

class MeshOptimizer {
  static constexpr size_t MaxColorLayers = Mesh::MaxColorLayers;
  static constexpr size_t MaxUVLayers = Mesh::MaxUVLayers;
  static constexpr size_t MaxSkinEntries = Mesh::MaxSkinEntries;

  const std::vector<Material>& materials;
  bool use_luvs;

  uint32_t color_count;
  uint32_t uv_count;

  struct Vertex {
    Vector3f co = {};
    std::array<Mesh::SkinBind, MaxSkinEntries> skin_ents = {};
    explicit Vertex(Connection& conn);
  };
  std::vector<Vertex> verts;

  struct Loop {
    Vector3f normal = {};
    std::array<Vector3f, MaxColorLayers> colors = {};
    std::array<Vector2f, MaxUVLayers> uvs = {};
    uint32_t vert = UINT32_MAX;
    uint32_t edge = UINT32_MAX;
    uint32_t face = UINT32_MAX;
    uint32_t link_loop_next = UINT32_MAX;
    uint32_t link_loop_prev = UINT32_MAX;
    uint32_t link_loop_radial_next = UINT32_MAX;
    uint32_t link_loop_radial_prev = UINT32_MAX;
    explicit Loop(Connection& conn, uint32_t color_count, uint32_t uv_count);
  };
  std::vector<Loop> loops;

  struct Edge {
    static constexpr size_t MaxLinkFaces = 8;
    IndexArray<2> verts;
    IndexArray<MaxLinkFaces> link_faces;
    bool is_contiguous = false;
    bool tag = false;
    explicit Edge(Connection& conn);
  };
  std::vector<Edge> edges;

  struct Face {
    Vector3f normal = {};
    Vector3f centroid = {};
    uint32_t material_index = UINT32_MAX;
    IndexArray<3> loops;
    explicit Face(Connection& conn);
  };
  std::vector<Face> faces;

  std::unordered_map<Vector3f, uint32_t> b_pos;
  std::unordered_map<Vector3f, uint32_t> b_norm;
  std::unordered_map<std::array<Mesh::SkinBind, MaxSkinEntries>, uint32_t> b_skin;
  std::unordered_map<Vector3f, uint32_t> b_color;
  std::unordered_map<Vector2f, uint32_t> b_uv;
  std::unordered_map<Vector2f, uint32_t> b_luv;

  uint32_t get_pos_idx(const Vertex& v) const;
  uint32_t get_norm_idx(const Loop& l) const;
  uint32_t get_skin_idx(const Vertex& v) const;
  uint32_t get_color_idx(const Loop& l, uint32_t cidx) const;
  uint32_t get_uv_idx(const Loop& l, uint32_t uidx) const;
  void sort_faces_by_skin_group(std::vector<uint32_t>& faces) const;
  std::pair<uint32_t, uint32_t> strip_next_loop(uint32_t prev_loop, uint32_t out_count) const;

  bool loops_contiguous(const Loop& la, const Loop& lb) const;
  bool splitable_edge(const Edge& e) const;
  Mesh::Surface generate_surface(std::vector<uint32_t>& island_faces, uint32_t mat_idx) const;

public:
  explicit MeshOptimizer(Connection& conn, const std::vector<Material>& materials, bool use_luvs);
  void optimize(Mesh& mesh, int max_skin_banks) const;
};

}