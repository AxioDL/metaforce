#ifndef SPECTER_TABLE_HPP
#define SPECTER_TABLE_HPP

#include "View.hpp"

namespace Specter
{

class Table : public View
{
public:
    Table(ViewResources& res, View& parentView);
};

}

#endif // SPECTER_TABLE_HPP
