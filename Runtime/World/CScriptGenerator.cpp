#include "CScriptGenerator.hpp"
#include "CStateManager.hpp"
#include "CWallCrawlerSwarm.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptGenerator::CScriptGenerator(TUniqueId uid, std::string_view name, const CEntityInfo& info, u32 spawnCount,
                                   bool reuseFollowers, const zeus::CVector3f& vec1, bool inheritXf, bool active,
                                   float minScale, float maxScale)
: CEntity(uid, info, active, name)
, x34_spawnCount(spawnCount)
, x38_24_reuseFollowers(reuseFollowers)
, x38_25_inheritTransform(inheritXf)
, x3c_offset(vec1)
, x48_minScale(minScale)
, x4c_maxScale(maxScale)
{
}

void CScriptGenerator::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptGenerator::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    return;
    if (msg == EScriptObjectMessage::SetToZero && GetActive() && !x20_conns.empty())
    {
        std::vector<TUniqueId> follows;
        follows.reserve(x20_conns.size());
        for (const SConnection& conn : x20_conns)
        {
            if (conn.x0_state != EScriptObjectState::Zero || conn.x4_msg != EScriptObjectMessage::Follow)
                continue;

            TUniqueId uid = stateMgr.GetIdForScript(conn.x8_objId);
            if (stateMgr.GetObjectById(uid) != nullptr)
                follows.push_back(uid);
        }

        std::vector<std::pair<TUniqueId, TEditorId>> activates;
        activates.reserve(x20_conns.size());

        for (const SConnection& conn : x20_conns)
        {
            if (conn.x0_state != EScriptObjectState::Zero)
                continue;

            TUniqueId uid = stateMgr.GetIdForScript(conn.x8_objId);

            if (conn.x4_msg != EScriptObjectMessage::Activate)
            {
                stateMgr.SendScriptMsgAlways(GetUniqueId(), uid, conn.x4_msg);
                continue;
            }

            if (stateMgr.GetObjectById(uid) != nullptr)
                activates.emplace_back(uid, conn.x8_objId);
        }

        for (u32 i = 0; i < x34_spawnCount; ++i)
        {
            if (activates.size() == 0 || follows.size() == 0)
                return;

            u32 activatesRand = 0.99f * (stateMgr.GetActiveRandom()->Float() * activates.size());
            u32 followsRand = 0.99f * (stateMgr.GetActiveRandom()->Float() * follows.size());

            for (u32 j = 0; j < activates.size(); ++j)
                if (TCastToConstPtr<CScriptSound>(stateMgr.GetObjectById(activates[j].first)))
                    activatesRand = j;

            std::pair<TUniqueId, TEditorId> idPair = activates[activatesRand];
            CEntity* activate = stateMgr.ObjectById(idPair.first);
            CEntity* follow = stateMgr.ObjectById(follows[followsRand]);

            if (!activate || !follow)
                break;

            bool oldGeneratingObject = stateMgr.GetIsGeneratingObject();
            stateMgr.SetIsGeneratingObject(true);
            std::pair<TEditorId, TUniqueId> objId = stateMgr.GenerateObject(idPair.second);
            stateMgr.SetIsGeneratingObject(oldGeneratingObject);

            if (objId.second != kInvalidUniqueId)
            {
                TCastToPtr<CActor> activateActor(stateMgr.ObjectById(idPair.first));
                TCastToPtr<CActor> followActor(follow);
                TCastToPtr<CWallCrawlerSwarm> wallCrawlerSwarm(follow);

                if (activateActor && wallCrawlerSwarm)
                {
                    if (x38_25_inheritTransform)
                        activateActor->SetTransform(wallCrawlerSwarm->GetTransform());
                    activateActor->SetTranslation(wallCrawlerSwarm->GetLastKilledOffset() + x3c_offset);
                }
                else if (activateActor && followActor)
                {
                    if (x38_25_inheritTransform)
                        activateActor->SetTransform(followActor->GetTransform());
                    activateActor->SetTranslation(followActor->GetTranslation() + x3c_offset);
                }
                else if (follow != nullptr)
                {
                    TCastToPtr<CActor> activateActor(stateMgr.ObjectById(objId.second));

                    if (activateActor && wallCrawlerSwarm)
                    {
                        if (x38_25_inheritTransform)
                            activateActor->SetTransform(wallCrawlerSwarm->GetTransform());
                        activateActor->SetTranslation(wallCrawlerSwarm->GetLastKilledOffset() + x3c_offset);
                    }
                    else if (activateActor && followActor)
                    {
                        if (x38_25_inheritTransform)
                            activateActor->SetTransform(followActor->GetTransform());
                        activateActor->SetTranslation(followActor->GetTranslation() + x3c_offset);
                    }

                    float rnd = stateMgr.GetActiveRandom()->Range(x48_minScale, x4c_maxScale);
                    CModelData* mData = followActor->ModelData();
                    if (mData && (mData->AnimationData() || mData->GetNormalModel()))
                        mData->SetScale(rnd * mData->GetScale());
                }

                stateMgr.SendScriptMsg(activate, GetUniqueId(), EScriptObjectMessage::Activate);

                activates.erase(std::find(activates.begin(), activates.end(), idPair));
                if (!x38_24_reuseFollowers)
                    follows.erase(std::find(follows.begin(), follows.end(), follows[followsRand]));
            }
        }
    }

    CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}
}
