#include "CMain.hpp"

#include "boo2/boo2.hpp"
#include "logvisor/logvisor.hpp"

namespace metaforce {

template <class App, class Win>
class Delegate : public boo2::DelegateBase<App, Win> {

};

}

int main(int argc, char** argv) noexcept {
  logvisor::RegisterConsoleLogger();
  return boo2::Application<metaforce::Delegate>::exec(argc, argv, "metaforce"sv);
}
