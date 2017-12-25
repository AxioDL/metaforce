#ifndef GUI_VECTORISATABLEVIEW_HPP
#define GUI_VECTORISATABLEVIEW_HPP

#include <QTableView>
#include <zeus/Math.hpp>

#if ZEUS_ARCH_X86_64 || ZEUS_ARCH_X86
#include "VectorISATableModelIntel.hpp"
#endif

class QSequentialAnimationGroup;

class VectorISATableView : public QTableView
{
    Q_OBJECT
#if ZEUS_ARCH_X86_64 || ZEUS_ARCH_X86
    VectorISATableModelIntel m_model;
#endif
    std::tuple<QWidget*, QSequentialAnimationGroup*, bool> m_backgroundWidgets[2] = {};
    int m_maxISA = 0;
public:
    VectorISATableView(QWidget* parent = Q_NULLPTR);
    void paintEvent(QPaintEvent* e) Q_DECL_OVERRIDE;
    QString getISAString() const { return m_model.getISAString(m_maxISA); }
};

#endif // GUI_VECTORISATABLEVIEW_HPP
