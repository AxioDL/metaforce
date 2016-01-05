#ifndef URDE_LOCALE_HPP
#define URDE_LOCALE_HPP

#include <Specter/Translator.hpp>

namespace URDE
{

std::vector<std::pair<const std::string*, const std::string*>> ListLocales();
const Specter::Locale* LookupLocale(const std::string& name);
const Specter::Locale* SystemLocaleOrEnglish();

}

#endif // URDE_LOCALE_HPP
