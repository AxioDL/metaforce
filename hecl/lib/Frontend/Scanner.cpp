#include "hecl/Frontend.hpp"

namespace hecl::Frontend {

int Scanner::_read() {
  if (m_sourceIt == m_source.end())
    return -1;
  return *m_sourceIt++;
}

bool Scanner::read() {
  if (ch == EOF)
    return false;
  if (ch == LF) {
    lastLine = std::move(currentLine);
    currentLine = std::string();
  }
  int c = _read();
  ch = char(c);
  if (ch == LF) {
    loc.line++;
    lfcol = loc.col;
    loc.col = 0;
  } else if (c != EOF) {
    currentLine += ch;
    loc.col++;
  }
  return c != EOF;
}

Token Scanner::next() {
  if (ch == EOF)
    return Token(Token::Kind::Eof, loc);

  char c = ch;
  int tline = loc.line;
  int tcol = loc.col;
  int tlfcol = lfcol;
  read();

  // skip comments and newlines
  while (c != EOF && (c == COMMENT || isspace(c))) {
    if (c == COMMENT) {
      while (c != LF && c != EOF) {
        tline = loc.line;
        tcol = loc.col;
        tlfcol = lfcol;
        c = ch;
        read();
      }
    }
    while (c != EOF && isspace(c)) {
      if (c == LF)
        return Token(Token::Kind::Lf, {tline - 1, tlfcol + 1});
      tline = loc.line;
      tcol = loc.col;
      tlfcol = lfcol;
      c = ch;
      read();
    }
  }

  Token::Kind kind = CharToTokenKind(c);
  if (kind != Token::Kind::None)
    return Token(kind, {tline, tcol});

  if (ch == EOF)
    return Token(Token::Kind::Eof, {tline, tcol});

  // ident or number
  if (isDigit(c)) {
    std::string buf;
    buf += c;
    while (isDigit(ch)) {
      buf += ch;
      read();
    }
    if (ch == '.') { // float
      buf += ch;
      read();
      while (isDigit(ch)) {
        buf += ch;
        read();
      }
    }
    return Token(Token::Kind::Number, std::move(buf), {tline, tcol});
  }

  if (isStartIdent(c)) {
    std::string buf;
    buf += c;
    while (isMidIdent(ch)) {
      buf += ch;
      read();
    }
    return Token(Token::Kind::Ident, std::move(buf), {tline, tcol});
  }

  error({tline, tcol}, "unexpected character '%c' (X'%02X')", chr(c), int(c));

  return Token(Token::Kind::None, {tline, tcol});
}

} // namespace hecl::Frontend
