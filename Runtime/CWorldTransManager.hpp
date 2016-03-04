#ifndef __PSHAG_CWORLDTRANSMANAGER_HPP__
#define __PSHAG_CWORLDTRANSMANAGER_HPP__

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

#endif // __PSHAG_CWORLDTRANSMANAGER_HPP__
