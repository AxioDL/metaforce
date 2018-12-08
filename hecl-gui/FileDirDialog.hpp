#pragma once

#include <QFileDialog>

class FileDirDialog : public QFileDialog {
  Q_OBJECT
public:
  FileDirDialog(QWidget* parent = nullptr) : QFileDialog(parent) { setFileMode(QFileDialog::Directory); }
};
