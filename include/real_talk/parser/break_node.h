
#ifndef _REAL_TALK_PARSER_BREAK_NODE_H_
#define _REAL_TALK_PARSER_BREAK_NODE_H_

#include <memory>
#include "real_talk/parser/stmt_node.h"

namespace real_talk {
namespace parser {

class BreakNode: public StmtNode {
 public:
  BreakNode(
      const real_talk::lexer::TokenInfo &start_token,
      const real_talk::lexer::TokenInfo &end_token)
      : start_token_(start_token),
        end_token_(end_token) {
  }

 private:
  virtual bool IsEqual(const Node &node) const override {
    const BreakNode &rhs = static_cast<const BreakNode&>(node);
    return start_token_ == rhs.start_token_
        && end_token_ == rhs.end_token_;
  }

  virtual void Print(std::ostream &stream) const override {
    stream << start_token_.GetValue() << end_token_.GetValue();
  }

  real_talk::lexer::TokenInfo start_token_;
  real_talk::lexer::TokenInfo end_token_;
};
}
}
#endif
