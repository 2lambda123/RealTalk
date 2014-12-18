
#ifndef _REAL_TALK_SEMANTIC_VAR_DEF_ANALYSIS_H_
#define _REAL_TALK_SEMANTIC_VAR_DEF_ANALYSIS_H_

#include <memory>
#include "real_talk/semantic/data_type.h"
#include "real_talk/semantic/data_storage.h"
#include "real_talk/semantic/def_analysis.h"

namespace real_talk {
namespace semantic {

class VarDefAnalysis: public DefAnalysis {
 public:
  VarDefAnalysis(std::unique_ptr<DataType> data_type,
                 std::unique_ptr<DataStorage> storage);
  const DataType &GetDataType() const override;

 private:
  virtual bool IsEqual(const DefAnalysis &rhs) const override;
  virtual void Print(std::ostream &stream) const override;

  std::unique_ptr<DataType> data_type_;
  std::unique_ptr<DataStorage> storage_;
};
}
}
#endif
