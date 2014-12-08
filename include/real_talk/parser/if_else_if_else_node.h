
#ifndef _REAL_TALK_PARSER_IF_ELSE_IF_ELSE_NODE_H_
#define _REAL_TALK_PARSER_IF_ELSE_IF_ELSE_NODE_H_

#include <memory>
#include <vector>
#include "real_talk/parser/else_if_node.h"
#include "real_talk/lexer/token_info.h"

namespace real_talk {
namespace parser {

class IfElseIfElseNode: public StmtNode {
 public:
  IfElseIfElseNode(std::unique_ptr<IfNode> if_node,
                   std::vector< std::unique_ptr<ElseIfNode> > else_if_nodes,
                   const real_talk::lexer::TokenInfo &else_token,
                   std::unique_ptr<ScopeNode> else_body)
      : if_else_if_(move(if_node), move(else_if_nodes)),
        else_token_(else_token),
        else_body_(move(else_body)) {
    assert(else_body_);
  }

 private:
  virtual bool IsEqual(const Node &node) const override {
    const IfElseIfElseNode &rhs = static_cast<const IfElseIfElseNode&>(node);
    return if_else_if_ == rhs.if_else_if_
        && else_token_ == rhs.else_token_
        && *else_body_ == *(rhs.else_body_);
  }

  virtual void Print(std::ostream &stream) const override {
    stream << if_else_if_ << ' ' << else_token_.GetValue() << ' ' << *else_body_;
  }

  IfElseIfNode if_else_if_;
  const real_talk::lexer::TokenInfo else_token_;
  std::unique_ptr<ScopeNode> else_body_;
};
}
}

#endif
