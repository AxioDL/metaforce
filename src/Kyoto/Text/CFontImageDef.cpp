#include <Kyoto/Alloc/CMemory.hpp>
#include <Kyoto/Text/CFontImageDef.hpp>

CFontImageDef::CFontImageDef(const TToken< CTexture >& texture, const CVector2f& cropFactor)
: mFPS(0.f), mTextures(1, texture), mCropFactor(cropFactor) {
  rstl::vector< TToken< CTexture > >::iterator it = mTextures.begin();
  for (; it != mTextures.end(); ++it) {
    it->Lock();
  }
}

CFontImageDef::CFontImageDef(const rstl::vector< TToken< CTexture > >& textures, float fps,
                             const CVector2f& cropFactor)
: mFPS(fps), mTextures(textures), mCropFactor(cropFactor) {
  rstl::vector< TToken< CTexture > >::iterator it = mTextures.begin();
  for (; it != mTextures.end(); ++it) {
    it->Lock();
  }
}

bool CFontImageDef::IsLoaded() const {
  for (int i = 0; i < mTextures.size(); ++i) {
    if (!mTextures[i].IsLoaded()) {
      return false;
    }
  }

  return true;
}

int CFontImageDef::CalculateBaseline() const {
  return (2.5f * GetHeight()) / 3.f;
}
