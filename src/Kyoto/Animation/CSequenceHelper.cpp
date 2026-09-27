#include "Kyoto/Animation/CSequenceHelper.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CTreeUtils.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CSequenceHelper::CSequenceHelper(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                 const CAnimSysContext& context)
: mContext(context) {
  mNodes.reserve(2);
  mNodes.push_back(a);
  mNodes.push_back(b);
}

CSequenceHelper::CSequenceHelper(const rstl::vector< rstl::rc_ptr< IMetaAnim > >& anims,
                                 const CAnimSysContext& context)
: mContext(context) {
  mNodes.reserve(anims.size());

  AUTO(it, anims.begin());
  AUTO(end, anims.end());
  for (; it != end; ++it) {
    const rstl::rc_ptr< IMetaAnim >& anim = *it;
    mNodes.push_back(
        anim->GetAnimationTree(context, CMetaAnimTreeBuildOrders::NoSpecialOrders()));
  }
}

CSequenceFundamentals CSequenceHelper::ComputeSequenceFundamentals() const {
  const int numNodes = mNodes.size();
  CCharAnimTime elapsed = CCharAnimTime::ZeroFlat();
  CVector3f offset = CVector3f::Zero();
  CCharAnimTime duration = CCharAnimTime::ZeroFlat();
  rstl::vector< CBoolPOINode > boolNodes;
  rstl::vector< CInt32POINode > int32Nodes;
  rstl::vector< CParticlePOINode > partNodes;
  rstl::vector< CSoundPOINode > soundNodes;

  if (numNodes > 0) {
    rstl::ncrc_ptr< CAnimTreeNode > node = Cast(mNodes[0]->Clone());
    for (int i = 0; i < numNodes; ++i) {
      CBoolPOINode boolArray[64];
      const uint numBools = node->GetBoolPOIList(CCharAnimTime::Infinity(), boolArray, 64, 0, 0);
      boolNodes.reserve(boolNodes.size() + numBools);
      for (uint j = 0; j < numBools; ++j) {
        CBoolPOINode& poi = boolArray[j];
        poi.SetTime(poi.GetTime() + duration);
        boolNodes.push_back(poi);
      }

      CInt32POINode int32Array[64];
      const uint numInt32s = node->GetInt32POIList(CCharAnimTime::Infinity(), int32Array, 64, 0, 0);
      int32Nodes.reserve(int32Nodes.size() + numInt32s);
      for (uint j = 0; j < numInt32s; ++j) {
        CInt32POINode& poi = int32Array[j];
        poi.SetTime(poi.GetTime() + duration);
        int32Nodes.push_back(poi);
      }

      CParticlePOINode partArray[64];
      const uint numParts =
          node->GetParticlePOIList(CCharAnimTime::Infinity(), partArray, 64, 0, 0);
      partNodes.reserve(partNodes.size() + numParts);
      for (uint j = 0; j < numParts; ++j) {
        CParticlePOINode& poi = partArray[j];
        poi.SetTime(poi.GetTime() + duration);
        partNodes.push_back(poi);
      }

      CSoundPOINode soundArray[64];
      const uint numSounds = node->GetSoundPOIList(CCharAnimTime::Infinity(), soundArray, 64, 0, 0);
      soundNodes.reserve(soundNodes.size() + numSounds);
      for (uint j = 0; j < numSounds; ++j) {
        CSoundPOINode& poi = soundArray[j];
        poi.SetTime(poi.GetTime() + duration);
        soundNodes.push_back(poi);
      }

      duration += node->GetTimeRemaining();
      CCharAnimTime remaining = node->GetTimeRemaining();
      while (remaining != CCharAnimTime::ZeroFlat() && !close_enough(remaining.GetSeconds(), 0.f)) {
        CCharAnimTime previous = remaining;
        CAdvancementResults result = node->AdvanceView(previous);
        rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
            node->Simplified();
        if (simplified.valid()) {
          node = Cast(*simplified);
        }

        remaining = result.GetRemainder();
        elapsed += previous - remaining;
        const rstl::optional_object< CVector3f > delta =
            result.GetAdvancementDeltas().GetOffsetDelta();
        offset += *delta;
      }

      if (i < numNodes - 1) {
        node = CTreeUtils::GetTransitionTree(node, Cast(mNodes[i + 1]->Clone()), mContext);
      }
    }
  }

  return CSequenceFundamentals(CSteadyStateAnimInfo(false, duration, offset), boolNodes, int32Nodes,
                               partNodes, soundNodes);
}
