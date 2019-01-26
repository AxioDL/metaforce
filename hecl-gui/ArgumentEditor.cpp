#include "ArgumentEditor.hpp"
#include "ui_ArgumentEditor.h"
#include <QSettings>
#include <QInputDialog>
#include <QDebug>

ArgumentEditor::ArgumentEditor(QWidget* parent)
: QDialog(parent)
, m_ui(new Ui::ArgumentEditor) {
  m_ui->setupUi(this);
  m_model.setStringList(QSettings().value("urde_arguments").toStringList());
  m_ui->argumentEditor->setModel(&m_model);
}

ArgumentEditor::~ArgumentEditor() {
  delete m_ui;
  m_ui = nullptr;
}

void ArgumentEditor::on_addButton_clicked() {
  QInputDialog input(this);
  int code = input.exec();
  if (code == DialogCode::Accepted) {
    QStringList list = m_model.stringList();
    list << input.textValue();
    m_model.setStringList(list);
  }
}

void ArgumentEditor::on_editButton_clicked() {
  QModelIndex index = m_ui->argumentEditor->currentIndex();
  if (!index.isValid())
    return;

  QInputDialog input(this);
  input.setTextValue(m_model.stringList().value(index.row()));
  int code = input.exec();
  if (code == DialogCode::Accepted) {
    QStringList list = m_model.stringList();
    list[index.row()] = input.textValue();
    m_model.setStringList(list);
  }
}

void ArgumentEditor::on_deleteButton_clicked() {
  QModelIndex index = m_ui->argumentEditor->currentIndex();
  if (index.isValid())
    m_model.removeRows(index.row(), 1);
}

void ArgumentEditor::on_buttonBox_clicked(QAbstractButton* button) {
  QDialogButtonBox* buttonBox = qobject_cast<QDialogButtonBox*>(sender());
  if (button == buttonBox->button(QDialogButtonBox::Ok)) {
    QSettings().setValue("urde_arguments", m_model.stringList());
    accept();
  } else {
    reject();
  }
}