#include "Specter/ScrollView.hpp"

namespace Specter
{

ScrollView::ScrollView(ViewResources& system, View& parentView, View& contentView)
: View(system, parentView), m_contentView(contentView)
{
}

}
