
#ifndef _REAL_TALK_PARSER_NOT_EQUAL_NODE_H_
#define _REAL_TALK_PARSER_NOT_EQUAL_NODE_H_

#include "real_talk/parser/expr_node.h"
#include "real_talk/parser/binary_expr_node.h"

namespace real_talk {
namespace parser {

class NotEqualNode: public ExprNode {
 public:
  NotEqualNode(
      const real_talk::lexer::TokenInfo &token,
      std::unique_ptr<ExprNode> left_operand,
      std::unique_ptr<ExprNode> right_operand)
      : binary_expr_(token, move(left_operand), move(right_operand)) {
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

  virtual void Accept(NodeVisitor &visitor) const override {
    visitor.VisitNotEqual(*this);
  }

 private:
  virtual bool IsEqual(const Node &node) const override {
    return binary_expr_ == static_cast<const NotEqualNode&>(node).binary_expr_;
  }

  virtual void Print(std::ostream &stream) const override {
    stream << binary_expr_;
  }

  BinaryExprNode binary_expr_;
};
}
}

#endif
