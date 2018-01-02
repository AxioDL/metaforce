#ifndef GUI_ERRORLABEL_HPP
#define GUI_ERRORLABEL_HPP

#include <QLabel>

class ErrorLabel : public QLabel
{
public:
    ErrorLabel(QWidget* parent = Q_NULLPTR) : QLabel(parent) {}
    void setText(const QString& str, bool success = false)
    {
        QPalette pal = QLabel::palette();
        if (success)
            pal.setColor(QPalette::WindowText, QColor(0,255,0));
        else
            pal.setColor(QPalette::WindowText, QColor(255,47,0));
        QLabel::setPalette(pal);
        QLabel::setText(str);
    }
};

#endif // GUI_ERRORLABEL_HPP
