#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QProcess>
#include <memory>
#include "DownloadManager.hpp"
class QTextEdit;
class QTextCharFormat;

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
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setTextTermFormatting(QTextEdit* textEdit, QString const & text);
    void parseEscapeSequence(int attribute, QListIterator< QString > & i, QTextCharFormat & textCharFormat, QTextCharFormat const & defaultTextCharFormat);
private slots:
    void onExtract();
    void onReturnPressed();
private:
    void initSlots();
    void onIndexDownloaded(const QStringList& index);
    void onBinaryDownloaded(const QString& file);
};

#endif // MAINWINDOW_HPP
