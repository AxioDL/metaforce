#ifndef SPECTER_DEFERREDWINDOWEVENTS_HPP
#define SPECTER_DEFERREDWINDOWEVENTS_HPP

#include <boo/boo.hpp>

namespace Specter
{

template <class Receiver>
struct DeferredWindowEvents : public boo::IWindowCallback
{
    Receiver& m_rec;
    std::mutex m_mt;
    DeferredWindowEvents(Receiver& rec) : m_rec(rec) {}

    bool m_destroyed = false;
    void destroyed()
    {
        m_destroyed = true;
    }

    bool m_hasResize = false;
    boo::SWindowRect m_latestResize;
    void resized(const boo::SWindowRect& rect)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_latestResize = rect;
        m_hasResize = true;
    }

    struct Command
    {
        enum class Type
        {
            MouseDown,
            MouseUp,
            MouseMove,
            MouseEnter,
            MouseLeave,
            Scroll,
            TouchDown,
            TouchUp,
            TouchMove,
            CharKeyDown,
            CharKeyUp,
            SpecialKeyDown,
            SpecialKeyUp,
            ModKeyDown,
            ModKeyUp
        } m_type;

        boo::SWindowCoord m_coord;
        boo::EMouseButton m_button;
        boo::EModifierKey m_mods;
        boo::SScrollDelta m_scroll;
        boo::STouchCoord m_tCoord;
        uintptr_t m_tid;
        unsigned long m_charcode;
        boo::ESpecialKey m_special;
        bool m_isRepeat;

        void dispatch(Receiver& rec) const
        {
            switch (m_type)
            {
            case Type::MouseDown:
                rec.mouseDown(m_coord, m_button, m_mods);
                break;
            case Type::MouseUp:
                rec.mouseUp(m_coord, m_button, m_mods);
                break;
            case Type::MouseMove:
                rec.mouseMove(m_coord);
                break;
            case Type::MouseEnter:
                rec.mouseEnter(m_coord);
                break;
            case Type::MouseLeave:
                rec.mouseLeave(m_coord);
                break;
            case Type::Scroll:
                rec.scroll(m_coord, m_scroll);
                break;
            case Type::TouchDown:
                rec.touchDown(m_tCoord, m_tid);
                break;
            case Type::TouchUp:
                rec.touchUp(m_tCoord, m_tid);
                break;
            case Type::TouchMove:
                rec.touchMove(m_tCoord, m_tid);
                break;
            case Type::CharKeyDown:
                rec.charKeyDown(m_charcode, m_mods, m_isRepeat);
                break;
            case Type::CharKeyUp:
                rec.charKeyUp(m_charcode, m_mods);
                break;
            case Type::SpecialKeyDown:
                rec.specialKeyDown(m_special, m_mods, m_isRepeat);
                break;
            case Type::SpecialKeyUp:
                rec.specialKeyUp(m_special, m_mods);
                break;
            case Type::ModKeyDown:
                rec.modKeyDown(m_mods, m_isRepeat);
                break;
            case Type::ModKeyUp:
                rec.modKeyUp(m_mods);
                break;
            default: break;
            }
        }

        Command(Type tp) : m_type(tp) {}
    };
    std::vector<Command> m_cmds;

    void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::MouseDown);
        m_cmds.back().m_coord = coord;
        m_cmds.back().m_button = button;
        m_cmds.back().m_mods = mods;
    }

    void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::MouseUp);
        m_cmds.back().m_coord = coord;
        m_cmds.back().m_button = button;
        m_cmds.back().m_mods = mods;
    }

    void mouseMove(const boo::SWindowCoord& coord)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::MouseMove);
        m_cmds.back().m_coord = coord;
    }

    void mouseEnter(const boo::SWindowCoord& coord)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::MouseEnter);
        m_cmds.back().m_coord = coord;
    }

    void mouseLeave(const boo::SWindowCoord& coord)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::MouseLeave);
        m_cmds.back().m_coord = coord;
    }

    void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::Scroll);
        m_cmds.back().m_coord = coord;
        m_cmds.back().m_scroll = scroll;
    }

    void touchDown(const boo::STouchCoord& coord, uintptr_t tid)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::TouchDown);
        m_cmds.back().m_tCoord = coord;
        m_cmds.back().m_tid = tid;
    }

    void touchUp(const boo::STouchCoord& coord, uintptr_t tid)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::TouchUp);
        m_cmds.back().m_tCoord = coord;
        m_cmds.back().m_tid = tid;
    }

    void touchMove(const boo::STouchCoord& coord, uintptr_t tid)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::TouchMove);
        m_cmds.back().m_tCoord = coord;
        m_cmds.back().m_tid = tid;
    }

    void charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::CharKeyDown);
        m_cmds.back().m_charcode = charCode;
        m_cmds.back().m_mods = mods;
        m_cmds.back().m_isRepeat = isRepeat;
    }

    void charKeyUp(unsigned long charCode, boo::EModifierKey mods)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::CharKeyUp);
        m_cmds.back().m_charcode = charCode;
        m_cmds.back().m_mods = mods;
    }

    void specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::SpecialKeyDown);
        m_cmds.back().m_special = key;
        m_cmds.back().m_mods = mods;
        m_cmds.back().m_isRepeat = isRepeat;
    }

    void specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::SpecialKeyUp);
        m_cmds.back().m_special = key;
        m_cmds.back().m_mods = mods;
    }

    void modKeyDown(boo::EModifierKey mod, bool isRepeat)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::ModKeyDown);
        m_cmds.back().m_mods = mod;
        m_cmds.back().m_isRepeat = isRepeat;
    }

    void modKeyUp(boo::EModifierKey mod)
    {
        std::unique_lock<std::mutex> lk(m_mt);
        m_cmds.emplace_back(Command::Type::ModKeyUp);
        m_cmds.back().m_mods = mod;
    }

    void dispatchEvents()
    {
        std::unique_lock<std::mutex> lk(m_mt);
        bool destroyed = m_destroyed;
        bool hasResize = m_hasResize;
        if (hasResize)
            m_hasResize = false;
        boo::SWindowRect latestResize = m_latestResize;
        std::vector<Command> cmds;
        m_cmds.swap(cmds);
        lk.unlock();

        if (destroyed)
        {
            m_rec.destroyed();
            return;
        }

        if (hasResize)
            m_rec.resized(latestResize, latestResize);

        for (const Command& cmd : cmds)
            cmd.dispatch(m_rec);
    }
};

}

#endif // SPECTER_DEFERREDWINDOWEVENTS_HPP
