#ifndef SPECTER_IVIEWMANAGER_HPP
#define SPECTER_IVIEWMANAGER_HPP

#include "Translator.hpp"

namespace Specter
{

struct IViewManager
{
public:
    virtual const Translator* getTranslator() const {return nullptr;}
    virtual std::string translateOr(const std::string& key, const char* vor) const
    {
        const Translator* trans = getTranslator();
        if (trans)
            return trans->translateOr(key, vor);
        return vor;
    }
};

}

#endif // SPECTER_IVIEWMANAGER_HPP
