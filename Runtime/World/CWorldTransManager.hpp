#ifndef __URDE_CWORLDTRANSMANAGER_HPP__
#define __URDE_CWORLDTRANSMANAGER_HPP__

namespace urde
{

class CWorldTransManager
{
    bool m_drawEnabled;
public:
    void DrawEnabled() const;
    void DrawDisabled() const;
    void Draw() const
    {
        if (m_drawEnabled)
            DrawEnabled();
        else
            DrawDisabled();
    }
};

}

#endif // __URDE_CWORLDTRANSMANAGER_HPP__
