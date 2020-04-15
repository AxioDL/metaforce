#pragma once

#include <QMenu>
#include <hecl/CVar.hpp>

namespace hecl {
struct CVarCommons;
} // namespace hecl

class QAction;

class LaunchMenu : public QMenu {
  Q_OBJECT
  hecl::CVarCommons& m_commons;

  QMenu m_apiMenu;
  QMenu m_msaaMenu;
  QMenu m_anisoMenu;
  QMenu m_experimentalMenu;
  QMenu m_developerMenu;

  QActionGroup m_apiGroup;
  QActionGroup m_msaaGroup;
  QActionGroup m_anisoGroup;

  QAction* m_developerMode = nullptr;
  QAction* m_developerMenuAction = nullptr;
  QAction* m_enableCheats = nullptr;
  QAction* m_variableDt = nullptr;

  QAction* m_debugOverlayAreaInfo = nullptr;
  QAction* m_debugOverlayPlayerInfo = nullptr;

  void initApiAction(const QString& action);
  void initMsaaAction(const QString& action);
  void initAnisoAction(const QString& action);
  void initDeepColor();
  void initDeveloperMode();
  void initCheats();
  void initExperimentalMenu();
  void initDeveloperMenu();
  void initCVarAction(QAction* action, hecl::CVar* cvar) const;

public:
  explicit LaunchMenu(hecl::CVarCommons& commons, QWidget* parent = Q_NULLPTR);
  ~LaunchMenu() override;

public slots:
  void apiTriggered();
  void msaaTriggered();
  void anisoTriggered();
  void deepColorTriggered();
  void developerModeTriggered();
  void cheatsTriggered();
  void editRuntimeArgs();
  void cvarTriggered();
};
