#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1 {
class CFlaahgraData {
  friend class CFlaahgra;
};

class CFlaahgra : public CPatterned {
public:
  DEFINE_PATTERNED(Flaahgra);

};
}