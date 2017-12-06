#include "FileDirDialog.hpp"
#include <QModelIndex>
#include <QPushButton>
#include <QTreeView>
#include <QListView>
#include <QEvent>
#include <QDir>

FileDirDialog::FileDirDialog(QWidget *parent)
    : QFileDialog(parent)
{
    m_selectedFiles.clear();

    this->setOption(QFileDialog::DontUseNativeDialog, true);
    this->setFileMode(QFileDialog::Directory);
    QList<QPushButton*> btns = this->findChildren<QPushButton*>();
    for (int i = 0; i < btns.size(); ++i)
    {
        QString text = btns[i]->text();
        if (text.toLower().contains("open") || text.toLower().contains("choose"))
        {
            m_btnOpen = btns[i];
            break;
        }
    }

    if (!m_btnOpen)
        return;

    m_btnOpen->installEventFilter(this);
    m_btnOpen->disconnect(SIGNAL(clicked()));
    connect(m_btnOpen, SIGNAL(clicked()), this, SLOT(chooseClicked()));


    m_listView = findChild<QListView*>("listView");
    if (m_listView) {
        m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

    m_treeView = findChild<QTreeView*>();
    if (m_treeView)
        m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

bool FileDirDialog::eventFilter( QObject* watched, QEvent* event )
{
    QPushButton *btn = qobject_cast<QPushButton*>(watched);
    if (btn && !btn->isEnabled() && event->type()==QEvent::EnabledChange)
        btn->setEnabled(true);

    return QWidget::eventFilter(watched, event);
}


void FileDirDialog::chooseClicked()
{
    QModelIndexList indexList = m_listView->selectionModel()->selectedIndexes();
    foreach (QModelIndex index, indexList)
        if (index.column( )== 0)
            m_selectedFiles.append(this->directory().absolutePath() + "/" + index.data().toString());

    QDialog::accept();
}

QStringList FileDirDialog::selectedFiles()
{
    return m_selectedFiles;
}
