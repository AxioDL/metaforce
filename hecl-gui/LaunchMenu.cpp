#include "LaunchMenu.hpp"
#include "hecl/CVarCommons.hpp"

extern hecl::BoolCVar* hecl::com_developer;

LaunchMenu::LaunchMenu(hecl::CVarCommons& commons, QWidget* parent)
: QMenu("Launch Menu", parent),
  m_commons(commons),
  m_apiMenu("Graphics API", this),
  m_msaaMenu("Anti-Aliasing", this),
  m_anisoMenu("Anisotropic Filtering", this),
  m_apiGroup(this),
  m_msaaGroup(this),
  m_anisoGroup(this)
{
#ifdef _WIN32
    initApiAction(QStringLiteral("D3D11"));
    initApiAction(QStringLiteral("Vulkan"));
    initApiAction(QStringLiteral("OpenGL"));
#elif defined(__APPLE__)
    initApiAction(QStringLiteral("Metal"));
    initApiAction(QStringLiteral("OpenGL"));
#else
    initApiAction(QStringLiteral("OpenGL"));
    initApiAction(QStringLiteral("Vulkan"));
#endif

    initMsaaAction(QStringLiteral("1"));
    initMsaaAction(QStringLiteral("2"));
    initMsaaAction(QStringLiteral("4"));
    initMsaaAction(QStringLiteral("8"));
    initMsaaAction(QStringLiteral("16"));

    initAnisoAction(QStringLiteral("1"));
    initAnisoAction(QStringLiteral("2"));
    initAnisoAction(QStringLiteral("4"));
    initAnisoAction(QStringLiteral("8"));
    initAnisoAction(QStringLiteral("16"));

    m_apiMenu.addActions(m_apiGroup.actions());
    m_msaaMenu.addActions(m_msaaGroup.actions());
    m_anisoMenu.addActions(m_anisoGroup.actions());
    addMenu(&m_apiMenu)->setToolTip(QStringLiteral("Graphics API to use for rendering graphics."));
    addMenu(&m_msaaMenu)->setToolTip(QStringLiteral("Select number of samples for MSAA rendering."));
    addMenu(&m_anisoMenu)->setToolTip(QStringLiteral("Select number of samples for anisotropic texture filtering."));

    initDeepColor();
    initDeveloperMode();
}

void LaunchMenu::initApiAction(const QString& action)
{
    QAction* act = m_apiGroup.addAction(action);
    connect(act, SIGNAL(triggered()), this, SLOT(apiTriggered()));
    act->setCheckable(true);
    if (!action.compare(QString::fromStdString(m_commons.getGraphicsApi()), Qt::CaseInsensitive))
        act->setChecked(true);
}

void LaunchMenu::initMsaaAction(const QString& action)
{
    QAction* act = m_msaaGroup.addAction(action);
    connect(act, SIGNAL(triggered()), this, SLOT(msaaTriggered()));
    act->setCheckable(true);
    if (!action.compare(QString::number(m_commons.getSamples()), Qt::CaseInsensitive))
        act->setChecked(true);
}

void LaunchMenu::initAnisoAction(const QString& action)
{
    QAction* act = m_anisoGroup.addAction(action);
    connect(act, SIGNAL(triggered()), this, SLOT(anisoTriggered()));
    act->setCheckable(true);
    if (!action.compare(QString::number(m_commons.getAnisotropy()), Qt::CaseInsensitive))
        act->setChecked(true);
}

void LaunchMenu::initDeepColor()
{
    QAction* act = addAction("Deep Color");
    act->setToolTip(QStringLiteral("Use 48-bit framebuffer for deep-color displays (e.g. 4K with HDR)."));
    act->setCheckable(true);
    act->setChecked(m_commons.getDeepColor());
    connect(act, SIGNAL(triggered()), this, SLOT(deepColorTriggered()));
}

void LaunchMenu::initDeveloperMode()
{
    QAction* act = addAction("&Developer Mode");
    act->setToolTip(QStringLiteral("Enable developer mode to use in-engine console commands."));
    act->setCheckable(true);
    act->setChecked(hecl::com_developer->value());
    connect(act, SIGNAL(triggered()), this, SLOT(developerModeTriggered()));
}

void LaunchMenu::apiTriggered()
{
    QString apiStr = qobject_cast<QAction*>(sender())->text();
    apiStr = apiStr.remove('&');
    m_commons.setGraphicsApi(apiStr.toStdString());
    m_commons.serialize();
}

void LaunchMenu::msaaTriggered()
{
    m_commons.setSamples(qobject_cast<QAction*>(sender())->text().toUInt());
    m_commons.serialize();
}

void LaunchMenu::anisoTriggered()
{
    m_commons.setAnisotropy(qobject_cast<QAction*>(sender())->text().toUInt());
    m_commons.serialize();
}

void LaunchMenu::deepColorTriggered()
{
    m_commons.setDeepColor(qobject_cast<QAction*>(sender())->isChecked());
    m_commons.serialize();
}

void LaunchMenu::developerModeTriggered()
{
    hecl::CVarManager::instance()->setDeveloperMode(qobject_cast<QAction*>(sender())->isChecked(), true);
    m_commons.serialize();
}
