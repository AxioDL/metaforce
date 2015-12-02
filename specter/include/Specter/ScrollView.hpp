#ifndef SPECTER_SCROLLVIEW_HPP
#define SPECTER_SCROLLVIEW_HPP

#include "View.hpp"

namespace Specter
{
class ViewResources;

class ScrollView : public View
{
    View& m_contentView;
public:
    ScrollView(ViewResources& res, View& parentView, View& contentView);
};

}

#endif // SPECTER_SCROLLVIEW_HPP
