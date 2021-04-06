#ifndef LAYERDIALOG_HPP
#define LAYERDIALOG_HPP

#include <QDialog>
#include <memory>

namespace Ui {
class LayerDialog;
} // namespace Ui

struct Layer {
  QString name;
  bool active;
};
class LayerDialog : public QDialog {
  Q_OBJECT
  std::unique_ptr<Ui::LayerDialog> m_ui;

public:
  explicit LayerDialog(QWidget* parent = nullptr);
  ~LayerDialog() override;

  void createLayerCheckboxes(QList<Layer> layers);
  QString getLayerBits() const;
};

#endif