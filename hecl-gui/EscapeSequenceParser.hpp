#ifndef GUI_ESCAPESEQUENCEPARSER_HPP
#define GUI_ESCAPESEQUENCEPARSER_HPP

#include <QString>
#include <QTextCharFormat>
#include <QTextCursor>

void ParseEscapeSequence(int attribute, QListIterator<QString>& i, QTextCharFormat& textCharFormat,
                         const QTextCharFormat& defaultTextCharFormat);

void ReturnInsert(QTextCursor& cur, const QString& text);

void ReturnInsert(QTextCursor& cur, const QString& text, const QTextCharFormat& format);

#endif // GUI_ESCAPESEQUENCEPARSER_HPP
