#include "CVarDialog.hpp"
#include "ui_CVarDialog.h"
#include <utility>
#include <array>

enum class CVarType {
  String,
  Boolean,
};

struct CVarItem {
  QString m_name;
  CVarType m_type;
  QVariant m_defaultValue;

  CVarItem(QString name, CVarType type, QVariant defaultValue)
  : m_name(std::move(name)), m_type(type), m_defaultValue(std::move(defaultValue)) {}
};

static std::array cvarList{
    CVarItem{QStringLiteral("tweak.game.FieldOfView"), CVarType::String, 55},
    CVarItem{QStringLiteral("debugOverlay.playerInfo"), CVarType::Boolean, false},
    CVarItem{QStringLiteral("debugOverlay.areaInfo"), CVarType::Boolean, false},
    // TODO expand
};

CVarDialog::CVarDialog(QWidget* parent) : QDialog(parent), m_ui(std::make_unique<Ui::CVarDialog>()) {
  m_ui->setupUi(this);
  QStringList list;
  for (const auto& item : cvarList) {
    list << item.m_name;
  }
  m_model.setStringList(list);
  m_ui->cvarList->setModel(&m_model);
  connect(m_ui->cvarList->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
          SLOT(handleSelectionChanged(QItemSelection)));
}

CVarDialog::~CVarDialog() = default;

void CVarDialog::on_buttonBox_accepted() {
  const QModelIndexList& list = m_ui->cvarList->selectionModel()->selectedIndexes();
  if (list.isEmpty()) {
    reject();
  } else {
    accept();
  }
}

void CVarDialog::on_buttonBox_rejected() { reject(); }

void CVarDialog::handleSelectionChanged(const QItemSelection& selection) {
  const QModelIndexList& list = selection.indexes();
  if (list.isEmpty()) {
    return;
  }
  const auto item = cvarList[(*list.begin()).row()];
  m_ui->valueStack->setCurrentIndex(static_cast<int>(item.m_type));
  if (item.m_type == CVarType::String) {
    m_ui->stringValueField->setText(item.m_defaultValue.toString());
  } else if (item.m_type == CVarType::Boolean) {
    m_ui->booleanValueField->setChecked(item.m_defaultValue.toBool());
  }
}

QString CVarDialog::textValue() {
  const QModelIndexList& list = m_ui->cvarList->selectionModel()->selectedIndexes();
  if (list.isEmpty()) {
    return QStringLiteral("");
  }
  const auto item = cvarList[(*list.begin()).row()];
  QVariant value;
  if (item.m_type == CVarType::String) {
    value = m_ui->stringValueField->text();
  } else if (item.m_type == CVarType::Boolean) {
    value = m_ui->booleanValueField->isChecked();
  }
  return QStringLiteral("+") + item.m_name + QStringLiteral("=") + value.toString();
}
