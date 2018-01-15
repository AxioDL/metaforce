#ifndef GUI_LAUNCHMENU_HPP
#define GUI_LAUNCHMENU_HPP

#include <QMenu>
namespace hecl { class CVarCommons; }

class LaunchMenu : public QMenu
{
    Q_OBJECT
    hecl::CVarCommons& m_commons;

    QMenu m_apiMenu;
    QMenu m_msaaMenu;
    QMenu m_anisoMenu;

    QActionGroup m_apiGroup;
    QActionGroup m_msaaGroup;
    QActionGroup m_anisoGroup;

    void initApiAction(const QString& action);
    void initMsaaAction(const QString& action);
    void initAnisoAction(const QString& action);
    void initDeveloperMode();

public:
    LaunchMenu(hecl::CVarCommons& commons, QWidget* parent = Q_NULLPTR);

public slots:
    void apiTriggered();
    void msaaTriggered();
    void anisoTriggered();
    void developerModeTriggered();
};

#endif // GUI_LAUNCHMENU_HPP
