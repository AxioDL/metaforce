#ifndef SPECTER_IVIEWMANAGER_HPP
#define SPECTER_IVIEWMANAGER_HPP

#include "Translator.hpp"
#include "SplitView.hpp"
#include <HECL/HECL.hpp>

namespace Specter
{
struct ISpaceController;

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

    virtual void deferSpaceSplit(ISpaceController* split, SplitView::Axis axis, int thisSlot,
                                 const boo::SWindowCoord& coord) {}

    virtual const std::vector<HECL::SystemString>* recentProjects() const {return nullptr;}
    virtual void pushRecentProject(const HECL::SystemString& path) {}

    virtual const std::vector<HECL::SystemString>* recentFiles() const {return nullptr;}
    virtual void pushRecentFile(const HECL::SystemString& path) {}
};

}

#endif // SPECTER_IVIEWMANAGER_HPP
