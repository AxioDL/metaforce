#ifndef SPECTER_SCROLLVIEW_HPP
#define SPECTER_SCROLLVIEW_HPP

#include "View.hpp"

namespace Specter
{
class ViewResources;

class ScrollView : public View
{
public:
    enum class Style
    {

    };

private:
    View* m_contentView = nullptr;
    int m_scroll[2] = {};

public:
    ScrollView(ViewResources& res, View& parentView);
    void setContentView(View* v)
    {
        m_contentView = v;
        updateSize();
    }

    void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll);

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_SCROLLVIEW_HPP
