#pragma once

#include <QFileDialog>

class FileDirDialog : public QFileDialog {
  Q_OBJECT
public:
  FileDirDialog(QWidget* parent = nullptr, QFileDialog::FileMode mode = QFileDialog::Directory) : QFileDialog(parent) {
    setFileMode(mode);
  }
};
