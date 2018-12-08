#pragma once

#include <QMainWindow>
#include <QProcess>
#include <QTextCursor>
#include <memory>
#include "DownloadManager.hpp"
#include "LaunchMenu.hpp"
#include "Common.hpp"
#include "hecl/Runtime.hpp"
#include "hecl/CVarCommons.hpp"
class QTextEdit;
class QTextCharFormat;
class QPushButton;
class QuaZip;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT
  hecl::Runtime::FileStoreManager m_fileMgr;
  hecl::CVarManager m_cvarManager;
  hecl::CVarCommons m_cvarCommons;
  Ui::MainWindow* m_ui;
  QTextCursor m_cursor;
  QString m_path;
  QString m_urdePath;
  QString m_heclPath;
  QProcess m_heclProc;
  DownloadManager m_dlManager;
  LaunchMenu m_launchMenu;
  QSettings m_settings;
  URDEVersion m_recommendedVersion;
  QPushButton* m_updateURDEButton;
  bool m_inContinueNote = false;

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();
  void setTextTermFormatting(const QString& text);
  void insertContinueNote(const QString& text);
private slots:
  void onExtract();
  void onExtractFinished(int exitCode);
  void onPackage();
  void onPackageFinished(int exitCode);
  void onLaunch();
  void onLaunchFinished(int exitCode);
  void doHECLTerminate();
  void onReturnPressed();
  void onDownloadPressed();
  void onUpdateURDEPressed();

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
};
