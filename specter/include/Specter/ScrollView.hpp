#ifndef SPECTER_SCROLLVIEW_HPP
#define SPECTER_SCROLLVIEW_HPP

#include "View.hpp"

namespace Specter
{
class ViewSystem;

class ScrollView : public View
{
    View& m_contentView;
public:
    ScrollView(ViewSystem& system, View& parentView, View& contentView);
};

}

#endif // SPECTER_SCROLLVIEW_HPP
