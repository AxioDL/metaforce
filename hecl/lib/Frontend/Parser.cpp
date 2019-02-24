#include "hecl/hecl.hpp"
#include "hecl/Frontend.hpp"

/* Syntatical token parsing system */

namespace hecl::Frontend {

/*
 * hecl   = { lf } call { lf { lf } call } { lf } .
 * call   = ident "(" [ expr { "," expr } ] ")" .
 * expr   = sum { ("+" | "-") sum } .
 * sum    = factor { ("*" | "/") factor } .
 * factor = value | "(" expr ")" .
 * value  = ( call [ "." ident ] )
 *        | [ "-" ] number
 *        .
 */

std::string IRNode::rep(int n, std::string_view s) {
  std::string buf;
  buf.reserve(n * s.size());
  for (int i = 0; i < n; i++)
    buf.append(s);
  return buf;
}

std::string IRNode::fmt(int level, bool stripUVAnims) const {
  std::string buf;
  auto indent = rep(level, "\t"sv);
  switch (kind) {
  case Kind::Call:
    if (stripUVAnims && (str == "Texture" || str == "TextureD" || str == "TextureN" || str == "TextureDN") &&
        children.size() >= 2) {
      auto it = children.begin();
      IRNode& uvNode = const_cast<IRNode&>(*++it);
      if (uvNode.str != "UV" && uvNode.str != "Normal" && uvNode.str != "View") {
        std::string replacementName(str);
        if (uvNode.str.back() == 'N' && replacementName.back() != 'N')
          replacementName += 'N';
        IRNode replacementNode(Kind::Call, std::move(replacementName), std::move(uvNode.children), loc);
        auto ret = replacementNode.fmt(level, false);
        uvNode.children = std::move(replacementNode.children);
        return ret;
      }
    }
    buf.append(indent);
    buf.append("Call "sv).append(str);
    if (!children.empty()) {
      buf.append(" {\n"sv);
      for (const IRNode& n : children) {
        buf.append(n.fmt(level + 1, stripUVAnims));
        buf.append("\n"sv);
      }
      buf.append(indent);
      buf.append("}"sv);
    }
    break;
  case Kind::Imm:
    buf.append(indent);
    buf.append("Imm "sv).append(hecl::Format("%f", val));
    break;
  case Kind::Binop:
    buf.append(indent);
    buf.append("Binop "sv).append(OpToStr(op)).append(" {\n"sv);
    buf.append(left->fmt(level + 1, stripUVAnims)).append("\n"sv);
    buf.append(right->fmt(level + 1, stripUVAnims)).append("\n"sv);
    buf.append(indent).append("}"sv);
    break;
  case Kind::Swizzle:
    buf.append(indent);
    buf.append("Swizzle \""sv).append(str);
    buf.append("\" {\n"sv);
    buf.append(left->fmt(level + 1, stripUVAnims)).append("\n"sv);
    buf.append(indent).append("}"sv);
    break;
  default:
    break;
  }
  return buf;
}

std::string IRNode::describe() const {
  std::vector<std::string> l;
  l.push_back("kind="s + KindToStr(kind).data());
  if (!str.empty())
    l.push_back("str="s + str);
  if (kind == Kind::Binop) {
    l.push_back("op="s + OpToStr(op).data());
    l.push_back("left="s + left->toString());
    l.push_back("right="s + right->toString());
  }
  if (kind == Kind::Swizzle)
    l.push_back("node="s + left->toString());
  if (kind == Kind::Call) {
    std::string str = "children=["s;
    for (auto it = children.begin(); it != children.end(); ++it) {
      str += it->toString();
      if (&*it != &children.back())
        str += ';';
    }
    str += ']';
    l.push_back(str);
  }

  std::string str = "IRNode["s;
  for (auto it = l.begin(); it != l.end(); ++it) {
    str += *it;
    if (&*it != &l.back())
      str += ';';
  }
  str += ']';
  return str;
}

void Parser::check(Token::Kind expected) {
  if (sym == expected)
    scan();
  else
    error("expected %s, was %s", Token::KindToStr(expected).data(), Token::KindToStr(sym).data());
}

IRNode Parser::call() {
  check(Token::Kind::Ident);
  std::string name = t.str;

  std::list<IRNode> args;
  check(Token::Kind::Lpar);
  if (sym == Token::Kind::Lpar || sym == Token::Kind::Ident || sym == Token::Kind::Number ||
      sym == Token::Kind::Minus) {
    args.push_back(expr());

    while (sym == Token::Kind::Comma) {
      scan();
      args.push_back(expr());
    }
  }

  if (sym != Token::Kind::Rpar)
    error("expected expr|rpar, was %s", Token::KindToStr(sym).data());
  else
    scan();
  return IRNode(IRNode::Kind::Call, std::move(name), std::move(args), t.loc);
}

bool Parser::imm(const IRNode& a, const IRNode& b) {
  return a.kind == IRNode::Kind::Imm && b.kind == IRNode::Kind::Imm;
}

IRNode Parser::expr() {
  IRNode node = sum();
  while (sym == Token::Kind::Plus || sym == Token::Kind::Minus) {
    scan();
    Token::Kind op = t.kind;
    IRNode right = sum();
    switch (op) {
    case Token::Kind::Plus:
      if (imm(node, right)) // constant folding
        return IRNode(IRNode::Kind::Imm, node.val + right.val, t.loc);
      else
        node = IRNode(IRNode::Op::Add, std::move(node), std::move(right), t.loc);
      break;
    case Token::Kind::Minus:
      if (imm(node, right)) // constant folding
        node = IRNode(IRNode::Kind::Imm, node.val - right.val, t.loc);
      else
        node = IRNode(IRNode::Op::Sub, std::move(node), std::move(right), t.loc);
      break;
    default:
      break;
    }
  }
  return node;
}

IRNode Parser::sum() {
  IRNode node = factor();
  while (sym == Token::Kind::Times || sym == Token::Kind::Div) {
    scan();
    Token::Kind op = t.kind;
    IRNode right = factor();
    switch (op) {
    case Token::Kind::Times:
      if (imm(node, right)) // constant folding
        node = IRNode(IRNode::Kind::Imm, node.val * right.val, t.loc);
      else
        node = IRNode(IRNode::Op::Mul, std::move(node), std::move(right), t.loc);
      break;
    case Token::Kind::Div:
      if (imm(node, right)) // constant folding
        node = IRNode(IRNode::Kind::Imm, node.val / right.val, t.loc);
      else
        node = IRNode(IRNode::Op::Div, std::move(node), std::move(right), t.loc);
      break;
    default:
      break;
    }
  }
  return node;
}

IRNode Parser::factor() {
  if (sym == Token::Kind::Lpar) {
    scan();
    IRNode node = expr();
    check(Token::Kind::Rpar);
    return node;
  } else
    return value();
}

IRNode Parser::value() {
  if (sym == Token::Kind::Number || sym == Token::Kind::Minus) {
    scan();
    bool neg = false;
    if (t.kind == Token::Kind::Minus) {
      neg = true;
      check(Token::Kind::Number);
    }
    float val = strtof(((neg ? "-"s : ""s) + t.str).c_str(), nullptr);
    return IRNode(IRNode::Kind::Imm, val, t.loc);
  } else if (sym == Token::Kind::Ident) {
    IRNode call = Parser::call();
    if (sym == Token::Kind::Period) {
      scan();
      check(Token::Kind::Ident);
      return IRNode(IRNode::Kind::Swizzle, std::string(t.str), std::move(call), t.loc);
    }
    return call;
  } else {
    error("expected number|call, was %s", Token::KindToStr(sym).data());
    return IRNode();
  }
}

std::list<IRNode> Parser::parse() {
  std::list<IRNode> result;
  scan();
  while (sym == Token::Kind::Lf)
    scan();
  result.push_back(call());
  while (sym == Token::Kind::Lf) {
    while (sym == Token::Kind::Lf)
      scan();
    if (sym != Token::Kind::Eof)
      result.push_back(call());
  }
  while (sym == Token::Kind::Lf)
    scan();
  check(Token::Kind::Eof);

  if (hecl::VerbosityLevel > 1)
    for (auto& res : result)
      printf("%s\n", res.toString().c_str());

  return result;
}

} // namespace hecl::Frontend
