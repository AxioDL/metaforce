#pragma once

#include <QMenu>
namespace hecl { struct CVarCommons; }

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
    void initDeepColor();
    void initDeveloperMode();

public:
    LaunchMenu(hecl::CVarCommons& commons, QWidget* parent = Q_NULLPTR);

public slots:
    void apiTriggered();
    void msaaTriggered();
    void anisoTriggered();
    void deepColorTriggered();
    void developerModeTriggered();
};

