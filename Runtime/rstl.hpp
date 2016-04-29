#ifndef __RSTL_HPP__
#define __RSTL_HPP__

#include <vector>
#include <stdlib.h>
#include "optional.hpp"

namespace rstl
{

template <typename T>
using optional_object = std::experimental::optional<T>;

/**
 * @brief Vector reserved on construction
 */
template <class T, size_t N>
class reserved_vector : public std::vector<T>
{
public:
    reserved_vector() {this->reserve(N);}
};

}

#endif // __RSTL_HPP__
