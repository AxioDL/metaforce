#ifndef FILEDIRDIALOG_HPP
#define FILEDIRDIALOG_HPP

#include <QFileDialog>

class FileDirDialog : public QFileDialog
{
    Q_OBJECT
public:
    FileDirDialog(QWidget* parent = nullptr) : QFileDialog(parent) { setFileMode(QFileDialog::Directory); }
};

#endif // FILEDIRDIALOG_HPP
