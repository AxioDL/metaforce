#ifndef __URDE_CLOGBOOKSCREEN_HPP__
#define __URDE_CLOGBOOKSCREEN_HPP__

#include "CInGameGuiManager.hpp"
#include "CPauseScreenBase.hpp"
#include "CArtifactDoll.hpp"

namespace urde
{
namespace MP1
{

class CLogBookScreen : public CPauseScreenBase
{
    rstl::reserved_vector<std::vector<u32>, 5> x19c_;
    std::vector<u32> x1f0_;
    rstl::reserved_vector<std::vector<u32>, 5> x200_;
    float x254_ = 0.f;
    std::unique_ptr<CArtifactDoll> x258_artifactDoll;
    u32 x25c_ = 0;
    union
    {
        struct
        {
            bool x260_24_ : 1;
            bool x260_25_ : 1;
            bool x260_26_ : 1;
        };
        s32 _dummy = 0;
    };

    void InitializeLogBook();

public:
    CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);
    ~CLogBookScreen();

    bool IsArtifactCategorySelected() const;

    bool VReady() const;
    void VActivate();
    u32 GetRightTableCount() const;
};

}
}

#endif // __URDE_CLOGBOOKSCREEN_HPP__
