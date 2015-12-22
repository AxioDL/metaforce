#include "Specter/Table.hpp"

namespace Specter
{

Table::Table(ViewResources& res, View& parentView)
: View(res, parentView)
{
    commitResources(res);
}

}
