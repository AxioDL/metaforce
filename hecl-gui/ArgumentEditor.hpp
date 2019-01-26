#pragma once

#include <QDialog>
#include <QStringListModel>

class QAbstractButton;

namespace Ui {
class ArgumentEditor;
}

class ArgumentEditor : public QDialog {
  Q_OBJECT
  Ui::ArgumentEditor* m_ui;
  QStringListModel m_model;
public:
  explicit ArgumentEditor(QWidget* parent = 0);
  virtual ~ArgumentEditor();

private slots:
  void on_addButton_clicked();
  void on_editButton_clicked();
  void on_deleteButton_clicked();
  void on_buttonBox_clicked(QAbstractButton*);
};

