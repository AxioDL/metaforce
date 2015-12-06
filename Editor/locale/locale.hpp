#ifndef RUDE_LOCALE_HPP
#define RUDE_LOCALE_HPP

#include <Specter/Translator.hpp>

namespace RUDE
{

std::vector<std::pair<const std::string*, const std::string*>> ListLocales();
const Specter::Locale* LookupLocale(const std::string& name);
const Specter::Locale* SystemLocaleOrEnglish();

}

#endif // RUDE_LOCALE_HPP
