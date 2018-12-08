#include "VectorISATableView.hpp"
#include <QHeaderView>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

void VectorISATableView::paintEvent(QPaintEvent* e) {
  QTableView* p = static_cast<QTableView*>(parent()->parent());
  int tableY = p->horizontalHeader()->height() + p->rowViewportPosition(6);
  int rHeight = rowHeight(0);
  for (int i = 0; i < 2; ++i) {
    int tableX;
    int width = 0;
    if (i == 0) {
      tableX = p->verticalHeader()->width() + columnViewportPosition(0);
      for (int j = 0; j <= m_maxISA; ++j)
        width += columnWidth(j);
    } else {
      tableX = p->verticalHeader()->width() + columnViewportPosition(m_maxISA + 1);
      for (int j = m_maxISA + 1; j < m_model.columnCount({}); ++j)
        width += columnWidth(j);
    }

    QWidget* w = std::get<0>(m_backgroundWidgets[i]);
    QSequentialAnimationGroup* animation = std::get<1>(m_backgroundWidgets[i]);
    QPropertyAnimation* pAnimation = static_cast<QPropertyAnimation*>(animation->animationAt(1));
    bool& running = std::get<2>(m_backgroundWidgets[i]);
    if (!running) {
      w->setGeometry(QRect(tableX, tableY, 0, rHeight));
      pAnimation->setStartValue(QRect(tableX, tableY, 0, rHeight));
      pAnimation->setEndValue(QRect(tableX, tableY, width, rHeight));
      animation->start();
      running = true;
    }
    if (animation->state() == QAbstractAnimation::State::Running)
      pAnimation->setEndValue(QRect(tableX, tableY, width, rHeight));
    else
      w->setGeometry(QRect(tableX, tableY, width, rHeight));
  }
  QTableView::paintEvent(e);
}

VectorISATableView::VectorISATableView(QWidget* parent) : QTableView(parent) {
  setModel(&m_model);

  for (int i = 0; i < m_model.columnCount({}); ++i) {
    if (m_model.data(m_model.index(0, i), Qt::UserRole).toBool())
      m_maxISA = i;
    else
      break;
  }

  horizontalHeader()->hide();
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  setFrameShape(QFrame::Shape::NoFrame);
  verticalHeader()->hide();
  verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
  setFocusPolicy(Qt::NoFocus);

  for (int i = 0; i < 2; ++i) {
    QWidget* w = new QWidget(parent);
    std::get<0>(m_backgroundWidgets[i]) = w;

    QPalette pal = palette();
    if (i == 0)
      pal.setColor(QPalette::Background, QColor::fromRgbF(0.f, 1.f, 0.f, 0.2f));
    else
      pal.setColor(QPalette::Background, QColor::fromRgbF(1.f, 0.f, 0.f, 0.2f));

    w->setAutoFillBackground(true);
    w->setPalette(pal);
    w->lower();
    w->show();

    QPropertyAnimation* animation = new QPropertyAnimation(w, "geometry", this);
    animation->setDuration(2000);
    animation->setEasingCurve(QEasingCurve::Type::InOutCubic);

    QSequentialAnimationGroup* seq = new QSequentialAnimationGroup(this);
    std::get<1>(m_backgroundWidgets[i]) = seq;
    seq->addPause(6 * 100);
    seq->addAnimation(animation);
  }
}
