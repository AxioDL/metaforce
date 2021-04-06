#pragma once

#include <memory>

#include <QMainWindow>
#include <QProcess>
#include <QTextCursor>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>

#include "Common.hpp"
#include "DownloadManager.hpp"

#include <hecl/CVarCommons.hpp>
#include <hecl/Runtime.hpp>

class QPushButton;
class QTextCharFormat;
class QTextEdit;
class QuaZip;

namespace Ui {
class MainWindow;
} // namespace Ui

class MainWindow : public QMainWindow {
  static const QStringList skUpdateTracks;
  Q_OBJECT
  std::unique_ptr<Ui::MainWindow> m_ui;
  hecl::Runtime::FileStoreManager m_fileMgr;
  hecl::CVarManager m_cvarManager;
  hecl::CVarCommons m_cvarCommons;
  QTextCursor m_cursor;
  QString m_path;
  QString m_urdePath;
  QString m_heclPath;
  QProcess m_heclProc;
  DownloadManager m_dlManager;
  QStringList m_warpSettings;
  QSettings m_settings;
  URDEVersion m_recommendedVersion;
  bool m_inContinueNote = false;
  QStringListModel m_launchOptionsModel;

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

  void setTextTermFormatting(const QString& text);
  void insertContinueNote(const QString& text);

private slots:
  void onExtract();
  void onExtractFinished(int exitCode, QProcess::ExitStatus);
  void onPackage();
  void onPackageFinished(int exitCode, QProcess::ExitStatus);
  void onLaunch();
  void onLaunchFinished(int exitCode, QProcess::ExitStatus);
  void doHECLTerminate();
  void onReturnPressed();
  void onDownloadPressed();
//  void onUpdateURDEPressed();
  void onUpdateTrackChanged(int index);

private:
  bool checkDownloadedBinary();
  void setPath(const QString& path);
  void initSlots();
  void onIndexDownloaded(const QStringList& index);
  void onBinaryDownloaded(QuaZip& file);
  void onBinaryFailed();
  void disableOperations();
  void enableOperations();
  bool isPackageComplete() const;
  void initOptions();
  void initGraphicsApiOption(QRadioButton* action, bool hidden, bool isDefault);
  void initNumberComboOption(QComboBox* action, hecl::CVar* cvar);
  void initCheckboxOption(QCheckBox* action, hecl::CVar* cvar);
};
