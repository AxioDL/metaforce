#pragma once

#include <QTableView>
#include "Common.hpp"

#if ZEUS_ARCH_X86_64 || ZEUS_ARCH_X86
#include "VectorISATableModelIntel.hpp"
#endif

class QSequentialAnimationGroup;

class VectorISATableView : public QTableView {
  Q_OBJECT
#if ZEUS_ARCH_X86_64 || ZEUS_ARCH_X86
  VectorISATableModelIntel m_model;
#endif
  std::tuple<QWidget*, QSequentialAnimationGroup*, bool> m_backgroundWidgets[2] = {};
  int m_maxISA = 0;

public:
  VectorISATableView(QWidget* parent = Q_NULLPTR);
  void paintEvent(QPaintEvent* e) Q_DECL_OVERRIDE;
  VectorISA getISA() const { return m_model.getISA(m_maxISA); }
  bool willRun(VectorISA visa) const { return m_model.willRun(visa); }
};
