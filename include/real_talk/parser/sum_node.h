
#ifndef _REAL_TALK_PARSER_SUM_NODE_H_
#define _REAL_TALK_PARSER_SUM_NODE_H_

#include "real_talk/parser/expr_node.h"
#include "real_talk/parser/binary_expr_node.h"
#include "real_talk/parser/node_visitor.h"

namespace real_talk {
namespace parser {

class SumNode: public ExprNode {
 public:
  SumNode(
      const real_talk::lexer::TokenInfo &token,
      std::unique_ptr<ExprNode> left_operand,
      std::unique_ptr<ExprNode> right_operand)
      : binary_expr_(token, move(left_operand), move(right_operand)) {
  }

  virtual void Accept(NodeVisitor &visitor) const override {
    visitor.VisitSum(*this);
  }

  const real_talk::lexer::TokenInfo &GetToken() const {
    return binary_expr_.GetToken();
  }

  const std::unique_ptr<ExprNode> &GetLeftOperand() const {
    return binary_expr_.GetLeftOperand();
  }

  const std::unique_ptr<ExprNode> &GetRightOperand() const {
    return binary_expr_.GetRightOperand();
  }

 private:
  virtual bool IsEqual(const Node &node) const override {
    return binary_expr_.IsEqual(
        static_cast<const SumNode&>(node).binary_expr_);
  }

  virtual void Print(std::ostream &stream) const override {
    binary_expr_.Print(stream);
  }

  BinaryExprNode binary_expr_;
};
}
}

#endif
