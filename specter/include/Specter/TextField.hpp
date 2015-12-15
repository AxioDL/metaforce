#ifndef SPECTER_TEXTFIELD_HPP
#define SPECTER_TEXTFIELD_HPP

#include "View.hpp"

namespace Specter
{

class TextField : public View
{
public:
    TextField(ViewResources& res, View& parentView);
};

}

#endif // SPECTER_TEXTFIELD_HPP
