#include "LayerDialog.hpp"
#include <QCheckBox>
#include "ui_LayerDialog.h"

LayerDialog::LayerDialog(QWidget* parent) : QDialog(parent), m_ui(std::make_unique<Ui::LayerDialog>()) {
  m_ui->setupUi(this);
}

LayerDialog::~LayerDialog() = default;

void LayerDialog::createLayerCheckboxes(QList<Layer> layers) {
  bool firstLayer = true;
  for (const auto& layer : layers) {
    QCheckBox* chkBox = new QCheckBox(layer.name);
    chkBox->setChecked(layer.active);
    if (firstLayer) {
      chkBox->setEnabled(false);
      firstLayer = false;
    }
    m_ui->checkboxLayout->addWidget(chkBox);
  }
}
QString LayerDialog::getLayerBits() const {
  QString layerBits;
  bool firstLayer = true;
  for (const auto& item : findChildren<QCheckBox*>()) {
    if (firstLayer) {
      layerBits += QLatin1String("1");
      firstLayer = false;
    } else {
      layerBits += item->isChecked() ? QStringLiteral("1") : QStringLiteral("0");
    }
  }
  return layerBits;
}
