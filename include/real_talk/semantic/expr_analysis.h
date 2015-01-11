
#ifndef _REAL_TALK_SEMANTIC_EXPR_ANALYSIS_H_
#define _REAL_TALK_SEMANTIC_EXPR_ANALYSIS_H_

#include <memory>
#include <iostream>
#include "real_talk/semantic/data_type.h"
#include "real_talk/semantic/value_type.h"

namespace real_talk {
namespace semantic {

class ExprAnalysis {
 public:
  ExprAnalysis(std::unique_ptr<DataType> data_type,
               ValueType value_type);
  const DataType &GetDataType() const;
  ValueType GetValueType() const;
  friend bool operator==(const ExprAnalysis &lhs,
                         const ExprAnalysis &rhs);
  friend std::ostream &operator<<(std::ostream &stream,
                                  const ExprAnalysis &analysis);

 private:
  std::unique_ptr<DataType> data_type_;
  ValueType value_type_;
};
}
}
#endif
