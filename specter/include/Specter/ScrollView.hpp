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

public:
    ScrollView(ViewResources& res, View& parentView);
    void setContentView(View* v)
    {
        m_contentView = v;
        updateSize();
    }
};

}

#endif // SPECTER_SCROLLVIEW_HPP
