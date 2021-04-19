#include "ModelViewer.hpp"

namespace metaforce {

void ModelViewer::View::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
  specter::View::resized(root, sub);
  m_scissorRect = sub;
}

} // namespace metaforce
