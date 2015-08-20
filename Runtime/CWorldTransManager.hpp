#ifndef __RETRO_CWORLDTRANSMANAGER_HPP__
#define __RETRO_CWORLDTRANSMANAGER_HPP__

namespace Retro
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

#endif // __RETRO_CWORLDTRANSMANAGER_HPP__
