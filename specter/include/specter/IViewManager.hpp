#ifndef SPECTER_IVIEWMANAGER_HPP
#define SPECTER_IVIEWMANAGER_HPP

#include "Translator.hpp"
#include "SplitView.hpp"
#include <hecl/hecl.hpp>

namespace specter
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

    virtual const std::vector<hecl::SystemString>* recentProjects() const {return nullptr;}
    virtual void pushRecentProject(const hecl::SystemString& path) {}

    virtual const std::vector<hecl::SystemString>* recentFiles() const {return nullptr;}
    virtual void pushRecentFile(const hecl::SystemString& path) {}
};

}

#endif // SPECTER_IVIEWMANAGER_HPP
