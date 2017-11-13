#ifndef URDE_LOCALE_HPP
#define URDE_LOCALE_HPP

#include <specter/Translator.hpp>

namespace urde
{

std::vector<std::pair<std::string_view, std::string_view>> ListLocales();
const specter::Locale* LookupLocale(std::string_view name);
const specter::Locale* SystemLocaleOrEnglish();

}

#endif // URDE_LOCALE_HPP
