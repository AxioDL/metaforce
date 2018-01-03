#ifndef GUI_ESCAPESEQUENCEPARSER_HPP
#define GUI_ESCAPESEQUENCEPARSER_HPP

#include <QString>
#include <QTextCharFormat>

void ParseEscapeSequence(int attribute, QListIterator<QString>& i, QTextCharFormat& textCharFormat,
                         const QTextCharFormat& defaultTextCharFormat);

#endif // GUI_ESCAPESEQUENCEPARSER_HPP
