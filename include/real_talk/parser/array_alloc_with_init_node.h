
#ifndef _REAL_TALK_PARSER_ARRAY_ALLOC_WITH_INIT_NODE_H_
#define _REAL_TALK_PARSER_ARRAY_ALLOC_WITH_INIT_NODE_H_

#include <boost/iterator/indirect_iterator.hpp>
#include <vector>
#include "real_talk/parser/array_alloc_node.h"
#include "real_talk/parser/base_array_alloc_node.h"

namespace real_talk {
namespace parser {

class ArrayAllocWithInitNode: public ArrayAllocNode {
 public:
  ArrayAllocWithInitNode(
      const real_talk::lexer::TokenInfo &op_token,
      std::unique_ptr<PrimitiveDataTypeNode> data_type,
      std::vector< std::unique_ptr<ArraySizeNode> > sizes,
      const real_talk::lexer::TokenInfo &values_start_token,
      std::vector< std::unique_ptr<ExprNode> > values,
      const std::vector< real_talk::lexer::TokenInfo > &value_separator_tokens,
      const real_talk::lexer::TokenInfo &values_end_token)
      : array_alloc_node_(op_token, move(data_type), move(sizes)),
        values_start_token_(values_start_token),
        values_(move(values)),
        value_separator_tokens_(value_separator_tokens),
        values_end_token_(values_end_token) {
    assert(!values_.empty());
    assert(value_separator_tokens_.size() == values_.size() - 1);
  }

  const std::vector< std::unique_ptr<ArraySizeNode> > &GetSizes()
      const override {
    return array_alloc_node_.GetSizes();
  }

  const std::unique_ptr<PrimitiveDataTypeNode> &GetDataType() const override {
    return array_alloc_node_.GetDataType();
  }

  const std::vector< std::unique_ptr<ExprNode> > &GetValues() const {
    return values_;
  }

  virtual void Accept(NodeVisitor &visitor) const override {
    visitor.VisitArrayAllocWithInit(*this);
  }

 private:
  virtual bool IsEqual(const Node &node) const override {
    const ArrayAllocWithInitNode &rhs =
        static_cast<const ArrayAllocWithInitNode&>(node);
    return array_alloc_node_ == rhs.array_alloc_node_
        && values_start_token_ == rhs.values_start_token_
        && values_end_token_ == rhs.values_end_token_
        && value_separator_tokens_.size() ==
        rhs.value_separator_tokens_.size()
        && std::equal(value_separator_tokens_.begin(),
                      value_separator_tokens_.end(),
                      rhs.value_separator_tokens_.begin())
        && values_.size() == rhs.values_.size()
        && std::equal(
            boost::make_indirect_iterator(values_.begin()),
            boost::make_indirect_iterator(values_.end()),
            boost::make_indirect_iterator(rhs.values_.begin()));
  }

  virtual void Print(std::ostream &stream) const override {
    stream << array_alloc_node_ << values_start_token_.GetValue();
    auto last_value_it = values_.end() - 1;
    auto value_separator_token_it = value_separator_tokens_.begin();

    for (auto value_it = values_.begin();
         value_it != last_value_it;
         ++value_it) {
      stream << **value_it << value_separator_token_it->GetValue() << ' ';
      ++value_separator_token_it;
    }

    stream << **last_value_it << values_end_token_.GetValue();
  }

  BaseArrayAllocNode array_alloc_node_;
  real_talk::lexer::TokenInfo values_start_token_;
  std::vector< std::unique_ptr<ExprNode> > values_;
  std::vector< real_talk::lexer::TokenInfo > value_separator_tokens_;
  real_talk::lexer::TokenInfo values_end_token_;
};
}
}
#endif
