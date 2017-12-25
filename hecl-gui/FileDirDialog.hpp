#ifndef FILEDIRDIALOG_HPP
#define FILEDIRDIALOG_HPP

#include <QFileDialog>
#include <QTreeWidget>
#include <QPushButton>
#include <QStringList>

class QPushButton;
class QTreeView;
class QListView;

class FileDirDialog : public QFileDialog
{
    Q_OBJECT
private:
    QListView* m_listView = nullptr;
    QTreeView* m_treeView = nullptr;
    QPushButton* m_btnOpen = nullptr;
    QStringList m_selectedFiles;

public slots:
    void chooseClicked();
public:
    FileDirDialog(QWidget* parent = nullptr);
    QStringList selectedFiles();
    bool eventFilter(QObject* watched, QEvent* event);
};

#endif // FILEDIRDIALOG_HPP
