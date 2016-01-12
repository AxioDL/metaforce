#ifndef URDE_RESOURCE_OUTLINER_HPP
#define URDE_RESOURCE_OUTLINER_HPP

#include "Space.hpp"
#include "ViewManager.hpp"
#include "Specter/PathButtons.hpp"

namespace URDE
{

class ResourceBrowser : public Space, public Specter::IPathButtonsBinding
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
        Value<Specter::SortDirection> sortDir = Specter::SortDirection::Ascending;
    } m_state;
    const Space::State& spaceState() const {return m_state;}

    HECL::ProjectPath m_path;
    std::vector<HECL::SystemString> m_comps;

    void pathButtonActivated(size_t idx);

    struct ResListingDataBind : Specter::ITableDataBinding, Specter::ITableStateBinding
    {
        ResourceBrowser& m_rb;

        struct Entry
        {
            HECL::SystemString m_path;
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

        void updateListing(const HECL::DirectoryEnumerator& dEnum)
        {
            m_entries.clear();
            m_entries.reserve(dEnum.size());

            for (const HECL::DirectoryEnumerator::Entry& d : dEnum)
            {
                m_entries.emplace_back();
                Entry& ent = m_entries.back();
                ent.m_path = d.m_path;
                HECL::SystemUTF8View nameUtf8(d.m_name);
                ent.m_name = nameUtf8.str();
                if (d.m_isDir)
                {
                    if (HECL::SearchForProject(d.m_path))
                        ent.m_type = m_projStr;
                    else
                        ent.m_type = m_dirStr;
                }
                else
                {
                    ent.m_type = m_fileStr;
                    ent.m_size = HECL::HumanizeNumber(d.m_fileSz, 7, nullptr, int(HECL::HNScale::AutoScale),
                                                      HECL::HNFlags::B | HECL::HNFlags::Decimal);
                }
            }

            m_needsUpdate = false;
        }

        bool m_needsUpdate = false;

        Specter::SortDirection getSort(size_t& cIdx) const
        {
            cIdx = size_t(m_rb.m_state.sortColumn);
            if (cIdx > 2)
                cIdx = 0;
            return m_rb.m_state.sortDir;
        }

        void setSort(size_t cIdx, Specter::SortDirection dir)
        {
            m_rb.m_state.sortDir = dir;
            m_needsUpdate = true;
        }

        void setSelectedRow(size_t rIdx)
        {
        }

        void rowActivated(size_t rIdx)
        {
        }

        ResListingDataBind(ResourceBrowser& rb, const Specter::IViewManager& vm)
        : m_rb(rb)
        {
            m_nameCol = vm.translateOr("name", "Name");
            m_typeCol = vm.translateOr("type", "Type");
            m_sizeCol = vm.translateOr("size", "Size");
            m_dirStr = vm.translateOr("directory", "Directory");
            m_projStr = vm.translateOr("hecl_project", "HECL Project");
            m_fileStr = vm.translateOr("file", "File");
        }

    } m_fileListingBind;

    struct View : Specter::View
    {
        ResourceBrowser& m_ro;
        Specter::ViewChild<std::unique_ptr<Specter::PathButtons>> m_pathButtons;
        Specter::ViewChild<std::unique_ptr<Specter::Table>> m_fileListing;

        View(ResourceBrowser& ro, Specter::ViewResources& res)
        : Specter::View(res, ro.m_vm.rootView()), m_ro(ro)
        {
            commitResources(res);
            m_pathButtons.m_view.reset(new Specter::PathButtons(res, *this, ro));
            m_fileListing.m_view.reset(new Specter::Table(res, *this, &ro.m_fileListingBind, &ro.m_fileListingBind, 3));
        }

        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    };
    std::unique_ptr<View> m_view;

public:
    ResourceBrowser(ViewManager& vm, Space* parent)
    : Space(vm, Class::ResourceBrowser, parent),
      m_fileListingBind(*this, vm)
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
        HECL::ProjectPath pp(*m_vm.project(), m_state.path);
        if (m_state.path.empty() || pp.getPathType() == HECL::ProjectPath::Type::None)
        {
            m_state.path = m_vm.project()->getProjectWorkingPath().getRelativePathUTF8();
            navigateToPath(HECL::ProjectPath(*m_vm.project(), m_state.path));
        }
        else
            navigateToPath(pp);
    }

    bool navigateToPath(const HECL::ProjectPath& path);

    Space* copy(Space* parent) const
    {
        return new ResourceBrowser(m_vm, parent, *this);
    }

    Specter::View* buildContentView(Specter::ViewResources& res)
    {
        m_view.reset(new View(*this, res));
        return m_view.get();
    }

    bool usesToolbar() const
    {
        return true;
    }
};

}

#endif // URDE_RESOURCE_OUTLINER_HPP
