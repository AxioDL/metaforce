#include "CMediumAllocPool.hpp"

namespace metaforce {

//CMediumAllocPool::CMediumAllocPool()
u32 CMediumAllocPool::GetTotalEntries() const {
  u32 ret = 0;
  for (const auto& puddle : x0_puddles) {
    ret += puddle.x1c_numEntries;
  }

  return ret;
}

u32 CMediumAllocPool::GetNumBlocksAvailable() const {
  u32 ret = 0;
  for (const auto& puddle : x0_puddles) {
    ret += puddle.x14_numBlocks;
  }

  return ret;
}

u32 CMediumAllocPool::GetNumAllocs() const {
  u32 ret = 0;
  for (const auto& puddle : x0_puddles) {
    ret += puddle.x18_numAllocs;
  }

  return ret;
}
}
