#ifndef __URDE_CSCRIPTSTREAMEDMUSIC_HPP__
#define __URDE_CSCRIPTSTREAMEDMUSIC_HPP__

#include "CEntity.hpp"

namespace urde
{

class CScriptStreamedMusic : public CEntity
{
    std::string x34_fileName;
    bool x44_b1;
    bool x45_fileNameValid;
    bool x46_b2;
    bool x47_b3;
    float x48_f1;
    float x4c_f2;
    u32 x50_w1;
    static bool ValidateFileName(const std::string& fileName);
    void StopStream(CStateManager& mgr);
    void StartStream(CStateManager& mgr);

public:
    CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info, const std::string& name,
                         bool, const std::string& fileName, bool, float, float, u32,
                         bool, bool);

    void Stop(CStateManager& mgr);
    void Play(CStateManager& mgr);
    void Accept(IVisitor& visitor);
    void PreThink(float, CStateManager&);
    void Think(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
};

}

#endif // __URDE_CSCRIPTSTREAMEDMUSIC_HPP__
