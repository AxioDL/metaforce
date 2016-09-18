#ifndef URDE_RESOURCE_OUTLINER_HPP
#define URDE_RESOURCE_OUTLINER_HPP

#include "Space.hpp"
#include "ViewManager.hpp"
#include "specter/PathButtons.hpp"

namespace urde
{

class ResourceBrowser : public Space, public specter::IPathButtonsBinding
{
    struct State : Space::State
    {
        DECL_YAML
        String<-1> path;
        Value<float> columnSplits[3] = {0.0f, 0.7f, 0.9f};
        enum class SortColumn
        {
            Name,
            Type,
            Size
        };
        Value<SortColumn> sortColumn = SortColumn::Name;
        Value<specter::SortDirection> sortDir = specter::SortDirection::Ascending;
    } m_state;
    const Space::State& spaceState() const {return m_state;}

    hecl::ProjectPath m_path;
    std::vector<hecl::SystemString> m_comps;

    void pathButtonActivated(size_t idx);

    struct ResListingDataBind : specter::ITableDataBinding, specter::ITableStateBinding
    {
        ResourceBrowser& m_rb;

        struct Entry
        {
            hecl::SystemString m_path;
            std::string m_name;
            std::string m_type;
            std::string m_size;
        };
        std::vector<Entry> m_entries;

        std::string m_nameCol;
        std::string m_typeCol;
        std::string m_sizeCol;

        std::string m_dirStr;
        std::string m_projStr;
        std::string m_fileStr;

        size_t columnCount() const {return 3;}
        size_t rowCount() const {return m_entries.size();}

        const std::string* header(size_t cIdx) const
        {
            switch (cIdx)
            {
            case 0:
                return &m_nameCol;
            case 1:
                return &m_typeCol;
            case 2:
                return &m_sizeCol;
            default: break;
            }
            return nullptr;
        }

        const std::string* cell(size_t cIdx, size_t rIdx) const
        {
            switch (cIdx)
            {
            case 0:
                return &m_entries.at(rIdx).m_name;
            case 1:
                return &m_entries.at(rIdx).m_type;
            case 2:
                return &m_entries.at(rIdx).m_size;
            default: break;
            }
            return nullptr;
        }

        bool columnSplitResizeAllowed() const {return true;}

        float getColumnSplit(size_t cIdx) const
        {
            return m_rb.m_state.columnSplits[cIdx];
        }

        void setColumnSplit(size_t cIdx, float split)
        {
            m_rb.m_state.columnSplits[cIdx] = split;
        }

        void updateListing(const hecl::DirectoryEnumerator& dEnum)
        {
            m_entries.clear();
            m_entries.reserve(dEnum.size());

            for (const hecl::DirectoryEnumerator::Entry& d : dEnum)
            {
                m_entries.emplace_back();
                Entry& ent = m_entries.back();
                ent.m_path = d.m_path;
                hecl::SystemUTF8View nameUtf8(d.m_name);
                ent.m_name = nameUtf8.str();
                if (d.m_isDir)
                {
                    if (hecl::SearchForProject(d.m_path))
                        ent.m_type = m_projStr;
                    else
                        ent.m_type = m_dirStr;
                }
                else
                {
                    ent.m_type = m_fileStr;
                    ent.m_size = hecl::HumanizeNumber(d.m_fileSz, 7, nullptr, int(hecl::HNScale::AutoScale),
                                                      hecl::HNFlags::B | hecl::HNFlags::Decimal);
                }
            }

            m_needsUpdate = false;
        }

        bool m_needsUpdate = false;

        specter::SortDirection getSort(size_t& cIdx) const
        {
            cIdx = size_t(m_rb.m_state.sortColumn);
            if (cIdx > 2)
                cIdx = 0;
            return m_rb.m_state.sortDir;
        }

        void setSort(size_t cIdx, specter::SortDirection dir)
        {
            m_rb.m_state.sortDir = dir;
            m_rb.m_state.sortColumn = State::SortColumn(cIdx);
            m_needsUpdate = true;
        }

        void setSelectedRow(size_t rIdx)
        {
        }

        void rowActivated(size_t rIdx)
        {
        }

        ResListingDataBind(ResourceBrowser& rb, const specter::IViewManager& vm)
        : m_rb(rb)
        {
            m_nameCol = vm.translateOr("name", "Name");
            m_typeCol = vm.translateOr("type", "Type");
            m_sizeCol = vm.translateOr("size", "Size");
            m_dirStr = vm.translateOr("directory", "Directory");
            m_projStr = vm.translateOr("hecl_project", "HECL Project");
            m_fileStr = vm.translateOr("file", "File");
        }

    } m_resListingBind;

    struct View : specter::View
    {
        ResourceBrowser& m_ro;
        specter::ViewChild<std::unique_ptr<specter::Table>> m_resListing;

        View(ResourceBrowser& ro, specter::ViewResources& res)
        : specter::View(res, ro.m_vm.rootView()), m_ro(ro)
        {
            m_resListing.m_view.reset(new specter::Table(res, *this, &ro.m_resListingBind, &ro.m_resListingBind, 3));
        }

        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseMove(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);

        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    };
    std::unique_ptr<View> m_view;

    std::unique_ptr<specter::PathButtons> m_pathButtons;

public:
    ResourceBrowser(ViewManager& vm, Space* parent)
    : Space(vm, Class::ResourceBrowser, parent),
      m_resListingBind(*this, vm)
    {
        reloadState();
    }
    ResourceBrowser(ViewManager& vm, Space* parent, const ResourceBrowser& other)
    : ResourceBrowser(vm, parent)
    {
        m_state = other.m_state;
        reloadState();
    }
    ResourceBrowser(ViewManager& vm, Space* parent, ConfigReader& r)
    : ResourceBrowser(vm, parent)
    {
        m_state.read(r);
        reloadState();
    }

    void reloadState()
    {
        hecl::ProjectPath pp(*m_vm.project(), m_state.path);
        if (m_state.path.empty() || pp.isNone())
        {
            m_state.path = m_vm.project()->getProjectWorkingPath().getRelativePathUTF8();
            navigateToPath(hecl::ProjectPath(*m_vm.project(), m_state.path));
        }
        else
            navigateToPath(pp);
    }

    void think()
    {
        if (m_resListingBind.m_needsUpdate)
            reloadState();
    }

    bool navigateToPath(const hecl::ProjectPath& path);

    Space* copy(Space* parent) const
    {
        return new ResourceBrowser(m_vm, parent, *this);
    }

    void buildToolbarView(specter::ViewResources &res, specter::Toolbar &tb)
    {
        m_pathButtons.reset(new specter::PathButtons(res, tb, *this, true));
        tb.push_back(m_pathButtons.get(), 1);
        reloadState();
    }

    specter::View* buildContentView(specter::ViewResources& res)
    {
        m_view.reset(new View(*this, res));
        return m_view.get();
    }

    bool usesToolbar() const
    {
        return true;
    }

    unsigned toolbarUnits() const
    {
        return 2;
    }
};

}

#endif // URDE_RESOURCE_OUTLINER_HPP
