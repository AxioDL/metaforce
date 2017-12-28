#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QProcess>
#include <memory>
#include "DownloadManager.hpp"
#include "Common.hpp"
class QTextEdit;
class QTextCharFormat;
class QPushButton;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Ui::MainWindow* m_ui;
    QString m_ansiString;
    QString m_path;
    QProcess m_heclProc;
    DownloadManager m_dlManager;
    QSettings m_settings;
    URDEVersion m_recommendedVersion;
    QPushButton* m_updateURDEButton;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setTextTermFormatting(QTextEdit* textEdit, QString const & text);
    void parseEscapeSequence(int attribute, QListIterator<QString>& i, QTextCharFormat& textCharFormat,
                             const QTextCharFormat& defaultTextCharFormat);
private slots:
    void onExtract();
    void onReturnPressed();
    void onDownloadPressed();
    void onUpdateURDEPressed();
private:
    void checkDownloadedBinary();
    void setPath(const QString& path);
    void initSlots();
    void onIndexDownloaded(const QStringList& index);
    void onBinaryDownloaded(const QString& file);
};

#endif // MAINWINDOW_HPP
