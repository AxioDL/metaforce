#include "LaunchMenu.hpp"
#include "hecl/CVarCommons.hpp"
#include "ArgumentEditor.hpp"
#include <QList>

LaunchMenu::LaunchMenu(hecl::CVarCommons& commons, QWidget* parent)
: QMenu(tr("Launch Menu"), parent)
, m_commons(commons)
, m_apiMenu(tr("Graphics API"), this)
, m_msaaMenu(tr("Anti-Aliasing"), this)
, m_anisoMenu(tr("Anisotropic Filtering"), this)
, m_experimentalMenu(tr("Experimental Features"), this)
, m_developerMenu(tr("Developer Options"), this)
, m_apiGroup(this)
, m_msaaGroup(this)
, m_anisoGroup(this) {
  setToolTipsVisible(true);
#ifdef _WIN32
  initApiAction(QStringLiteral("D3D11"));
  initApiAction(QStringLiteral("Vulkan"));
#elif defined(__APPLE__)
  initApiAction(QStringLiteral("Metal"));
#else
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
  initExperimentalMenu();
  initDeveloperMenu();
  addMenu(&m_apiMenu)->setToolTip(QString::fromUtf8(m_commons.m_graphicsApi->rawHelp().data()));
  addMenu(&m_msaaMenu)->setToolTip(QString::fromUtf8(m_commons.m_drawSamples->rawHelp().data()));
  addMenu(&m_anisoMenu)->setToolTip(QString::fromUtf8(m_commons.m_texAnisotropy->rawHelp().data()));
  addMenu(&m_experimentalMenu);
  m_developerMenuAction = addMenu(&m_developerMenu);
  m_developerMenuAction->setVisible(hecl::com_developer->toBoolean());
  const QAction* argumentEditor = addAction(tr("Edit Runtime Arguments"));
  connect(argumentEditor, &QAction::triggered, this, &LaunchMenu::editRuntimeArgs);
  initDeepColor();
  initDeveloperMode();
  initCheats();
}

LaunchMenu::~LaunchMenu() = default;

void LaunchMenu::initApiAction(const QString& action) {
  QAction* act = m_apiGroup.addAction(action);
  connect(act, &QAction::triggered, this, &LaunchMenu::apiTriggered);
  act->setCheckable(true);
  if (action.compare(QString::fromStdString(m_commons.getGraphicsApi()), Qt::CaseInsensitive) == 0) {
    act->setChecked(true);
  }
}

void LaunchMenu::initMsaaAction(const QString& action) {
  QAction* act = m_msaaGroup.addAction(action);
  connect(act, &QAction::triggered, this, &LaunchMenu::msaaTriggered);
  act->setCheckable(true);
  if (action.compare(QString::number(m_commons.getSamples()), Qt::CaseInsensitive) == 0) {
    act->setChecked(true);
  }
}

void LaunchMenu::initAnisoAction(const QString& action) {
  QAction* act = m_anisoGroup.addAction(action);
  connect(act, &QAction::triggered, this, &LaunchMenu::anisoTriggered);
  act->setCheckable(true);
  if (action.compare(QString::number(m_commons.getAnisotropy()), Qt::CaseInsensitive) == 0) {
    act->setChecked(true);
  }
}

void LaunchMenu::initDeepColor() {
  QAction* act = addAction(tr("Deep Color"));
  act->setToolTip(QString::fromUtf8(m_commons.m_deepColor->rawHelp().data()));
  act->setCheckable(true);
  act->setChecked(m_commons.getDeepColor());
  connect(act, &QAction::triggered, this, &LaunchMenu::deepColorTriggered);
}

void LaunchMenu::initDeveloperMode() {
  m_developerMode = addAction(tr("Developer Mode"));
  m_developerMode->setToolTip(QString::fromUtf8(hecl::com_developer->rawHelp().data()));
  m_developerMode->setCheckable(true);
  m_developerMode->setChecked(hecl::com_developer->toBoolean());
  connect(m_developerMode, &QAction::triggered, this, &LaunchMenu::developerModeTriggered);
}

void LaunchMenu::initCheats() {
  m_enableCheats = addAction(tr("Enable Cheats"));
  m_enableCheats->setToolTip(QString::fromUtf8(hecl::com_enableCheats->rawHelp().data()));
  m_enableCheats->setCheckable(true);
  m_enableCheats->setChecked(hecl::com_enableCheats->toBoolean());
  connect(m_enableCheats, &QAction::triggered, this, &LaunchMenu::cheatsTriggered);
}

void LaunchMenu::initExperimentalMenu() {
  initCVarAction(m_experimentalMenu.addAction(tr("Variable delta time")), m_commons.m_variableDt);
}

void LaunchMenu::initDeveloperMenu() {
  initCVarAction(m_developerMenu.addAction(tr("Area Info Overlay")), m_commons.m_debugOverlayAreaInfo);
  initCVarAction(m_developerMenu.addAction(tr("Player Info Overlay")), m_commons.m_debugOverlayPlayerInfo);
  initCVarAction(m_developerMenu.addAction(tr("World Info Overlay")), m_commons.m_debugOverlayWorldInfo);
  initCVarAction(m_developerMenu.addAction(tr("Show Frame Counter")), m_commons.m_debugOverlayShowFrameCounter);
  initCVarAction(m_developerMenu.addAction(tr("Show In-Game time")), m_commons.m_debugOverlayShowInGameTime);
  initCVarAction(m_developerMenu.addAction(tr("Show Resource Stats")), m_commons.m_debugOverlayShowResourceStats);
}

void LaunchMenu::apiTriggered() {
  QString apiStr = qobject_cast<QAction*>(sender())->text();
  apiStr = apiStr.remove(QLatin1Char{'&'});
  m_commons.setGraphicsApi(apiStr.toStdString());
  m_commons.serialize();
}

void LaunchMenu::msaaTriggered() {
  m_commons.setSamples(qobject_cast<QAction*>(sender())->text().toUInt());
  m_commons.serialize();
}

void LaunchMenu::anisoTriggered() {
  m_commons.setAnisotropy(qobject_cast<QAction*>(sender())->text().toUInt());
  m_commons.serialize();
}

void LaunchMenu::deepColorTriggered() {
  m_commons.setDeepColor(qobject_cast<QAction*>(sender())->isChecked());
  m_commons.serialize();
}

void LaunchMenu::developerModeTriggered() {
  const bool isChecked = qobject_cast<QAction*>(sender())->isChecked();

  if (hecl::com_enableCheats->toBoolean() && !isChecked) {
    m_enableCheats->setChecked(false);
  }
  m_developerMenuAction->setVisible(isChecked);

  hecl::CVarManager::instance()->setDeveloperMode(isChecked, true);
  m_commons.serialize();
}

void LaunchMenu::cheatsTriggered() {
  const bool isChecked = qobject_cast<QAction*>(sender())->isChecked();

  if (!hecl::com_developer->toBoolean() && isChecked) {
    m_developerMode->setChecked(true);
    m_developerMenuAction->setVisible(true);
  }

  hecl::CVarManager::instance()->setCheatsEnabled(isChecked, true);
  m_commons.serialize();
}

void LaunchMenu::editRuntimeArgs() {
  ArgumentEditor editor(this);
  editor.exec();
}

void LaunchMenu::initCVarAction(QAction* action, hecl::CVar* cvar) const {
  action->setData(QString::fromUtf8(cvar->name().data()));
  action->setToolTip(QString::fromUtf8(cvar->rawHelp().data()));
  action->setCheckable(true);
  action->setChecked(cvar->toBoolean());
  connect(action, &QAction::triggered, this, &LaunchMenu::cvarTriggered);
}

void LaunchMenu::cvarTriggered() {
  auto* action = qobject_cast<QAction*>(sender());
  hecl::CVar* cVar = hecl::CVarManager::instance()->findCVar(action->data().toString().toStdString());
  if (cVar != nullptr) {
    cVar->fromBoolean(action->isChecked());
    m_commons.serialize();
  }
}
