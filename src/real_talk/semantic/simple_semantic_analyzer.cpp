
#include <boost/functional/hash.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include <vector>
#include <string>
#include <utility>
#include <exception>
#include <functional>
#include "real_talk/lexer/lexer.h"
#include "real_talk/util/errors.h"
#include "real_talk/parser/parser.h"
#include "real_talk/parser/file_parser.h"
#include "real_talk/parser/node_visitor.h"
#include "real_talk/parser/stmt_node.h"
#include "real_talk/parser/sum_node.h"
#include "real_talk/parser/sub_node.h"
#include "real_talk/parser/not_node.h"
#include "real_talk/parser/and_node.h"
#include "real_talk/parser/or_node.h"
#include "real_talk/parser/pre_inc_node.h"
#include "real_talk/parser/pre_dec_node.h"
#include "real_talk/parser/mul_node.h"
#include "real_talk/parser/div_node.h"
#include "real_talk/parser/var_def_with_init_node.h"
#include "real_talk/parser/var_def_without_init_node.h"
#include "real_talk/parser/func_def_with_body_node.h"
#include "real_talk/parser/func_def_without_body_node.h"
#include "real_talk/parser/arg_def_node.h"
#include "real_talk/parser/program_node.h"
#include "real_talk/parser/expr_node.h"
#include "real_talk/parser/array_alloc_with_init_node.h"
#include "real_talk/parser/array_alloc_without_init_node.h"
#include "real_talk/parser/id_node.h"
#include "real_talk/parser/expr_stmt_node.h"
#include "real_talk/parser/int_node.h"
#include "real_talk/parser/long_node.h"
#include "real_talk/parser/char_node.h"
#include "real_talk/parser/string_node.h"
#include "real_talk/parser/double_node.h"
#include "real_talk/parser/bool_node.h"
#include "real_talk/parser/subscript_node.h"
#include "real_talk/parser/assign_node.h"
#include "real_talk/parser/greater_or_equal_node.h"
#include "real_talk/parser/less_or_equal_node.h"
#include "real_talk/parser/less_node.h"
#include "real_talk/parser/greater_node.h"
#include "real_talk/parser/equal_node.h"
#include "real_talk/parser/not_equal_node.h"
#include "real_talk/parser/call_node.h"
#include "real_talk/parser/pre_test_loop_node.h"
#include "real_talk/parser/if_else_if_node.h"
#include "real_talk/parser/if_else_if_else_node.h"
#include "real_talk/parser/int_data_type_node.h"
#include "real_talk/parser/long_data_type_node.h"
#include "real_talk/parser/double_data_type_node.h"
#include "real_talk/parser/char_data_type_node.h"
#include "real_talk/parser/string_data_type_node.h"
#include "real_talk/parser/bool_data_type_node.h"
#include "real_talk/parser/void_data_type_node.h"
#include "real_talk/parser/array_data_type_node.h"
#include "real_talk/parser/import_node.h"
#include "real_talk/parser/break_node.h"
#include "real_talk/parser/continue_node.h"
#include "real_talk/parser/negative_node.h"
#include "real_talk/parser/return_value_node.h"
#include "real_talk/parser/return_without_value_node.h"
#include "real_talk/semantic/simple_semantic_analyzer.h"
#include "real_talk/semantic/semantic_problems.h"
#include "real_talk/semantic/array_data_type.h"
#include "real_talk/semantic/string_data_type.h"
#include "real_talk/semantic/char_data_type.h"
#include "real_talk/semantic/int_data_type.h"
#include "real_talk/semantic/long_data_type.h"
#include "real_talk/semantic/double_data_type.h"
#include "real_talk/semantic/bool_data_type.h"
#include "real_talk/semantic/void_data_type.h"
#include "real_talk/semantic/int_lit.h"
#include "real_talk/semantic/long_lit.h"
#include "real_talk/semantic/double_lit.h"
#include "real_talk/semantic/string_lit.h"
#include "real_talk/semantic/char_lit.h"
#include "real_talk/semantic/bool_lit.h"
#include "real_talk/semantic/var_def_analysis.h"
#include "real_talk/semantic/arg_def_analysis.h"
#include "real_talk/semantic/func_def_analysis.h"
#include "real_talk/semantic/import_file_searcher.h"
#include "real_talk/semantic/lit_parser.h"
#include "real_talk/semantic/data_type_visitor.h"
#include "real_talk/semantic/data_type_converter.h"

using std::vector;
using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::unordered_map;
using std::unordered_set;
using std::runtime_error;
using std::pair;
using std::make_pair;
using std::move;
using std::exception;
using boost::format;
using boost::adaptors::reverse;
using boost::filesystem::path;
using boost::filesystem::filesystem_error;
using real_talk::lexer::Token;
using real_talk::lexer::TokenInfo;
using real_talk::lexer::UnexpectedCharError;
using real_talk::parser::ExprNode;
using real_talk::parser::StmtNode;
using real_talk::parser::AndNode;
using real_talk::parser::ArrayAllocWithoutInitNode;
using real_talk::parser::ArrayAllocWithInitNode;
using real_talk::parser::AssignNode;
using real_talk::parser::BoolNode;
using real_talk::parser::BreakNode;
using real_talk::parser::CallNode;
using real_talk::parser::CharNode;
using real_talk::parser::ContinueNode;
using real_talk::parser::DivNode;
using real_talk::parser::DoubleNode;
using real_talk::parser::EqualNode;
using real_talk::parser::ExprStmtNode;
using real_talk::parser::FuncDefWithBodyNode;
using real_talk::parser::FuncDefWithoutBodyNode;
using real_talk::parser::GreaterNode;
using real_talk::parser::GreaterOrEqualNode;
using real_talk::parser::IfElseIfElseNode;
using real_talk::parser::IfElseIfNode;
using real_talk::parser::ElseIfNode;
using real_talk::parser::ImportNode;
using real_talk::parser::IntNode;
using real_talk::parser::LessNode;
using real_talk::parser::LessOrEqualNode;
using real_talk::parser::LongNode;
using real_talk::parser::MulNode;
using real_talk::parser::NegativeNode;
using real_talk::parser::NotEqualNode;
using real_talk::parser::NotNode;
using real_talk::parser::OrNode;
using real_talk::parser::PreDecNode;
using real_talk::parser::PreIncNode;
using real_talk::parser::PreTestLoopNode;
using real_talk::parser::ProgramNode;
using real_talk::parser::StringNode;
using real_talk::parser::SubscriptNode;
using real_talk::parser::SubNode;
using real_talk::parser::SumNode;
using real_talk::parser::VarDefWithoutInitNode;
using real_talk::parser::VarDefWithInitNode;
using real_talk::parser::IdNode;
using real_talk::parser::IntDataTypeNode;
using real_talk::parser::LongDataTypeNode;
using real_talk::parser::DoubleDataTypeNode;
using real_talk::parser::CharDataTypeNode;
using real_talk::parser::StringDataTypeNode;
using real_talk::parser::BoolDataTypeNode;
using real_talk::parser::VoidDataTypeNode;
using real_talk::parser::ArrayDataTypeNode;
using real_talk::parser::DataTypeNode;
using real_talk::parser::ReturnValueNode;
using real_talk::parser::ReturnNode;
using real_talk::parser::ReturnWithoutValueNode;
using real_talk::parser::DefNode;
using real_talk::parser::VarDefNode;
using real_talk::parser::ArgDefNode;
using real_talk::parser::LitNode;
using real_talk::parser::BranchNode;
using real_talk::parser::FileParser;
using real_talk::parser::BoundedArraySizeNode;
using real_talk::parser::UnboundedArraySizeNode;
using real_talk::parser::ArrayAllocNode;
using real_talk::parser::UnexpectedTokenError;
using real_talk::parser::FuncDefNode;
using real_talk::util::FileNotFoundError;
using real_talk::util::IOError;

namespace real_talk {
namespace semantic {

class SimpleSemanticAnalyzer::Impl: public real_talk::parser::NodeVisitor {
 public:
  Impl(
      std::shared_ptr<real_talk::parser::ProgramNode> program,
      const boost::filesystem::path &absolute_file_path,
      const real_talk::parser::FileParser &file_parser,
      const ImportFileSearcher &import_file_searcher,
      const LitParser &lit_parser,
      const DataTypeConverter &data_type_converter);
  SemanticAnalysis Analyze();
  virtual void VisitAnd(const real_talk::parser::AndNode &node) override;
  virtual void VisitArrayAllocWithoutInit(
      const real_talk::parser::ArrayAllocWithoutInitNode &node) override;
  virtual void VisitArrayAllocWithInit(
      const real_talk::parser::ArrayAllocWithInitNode &node) override;
  virtual void VisitAssign(const real_talk::parser::AssignNode &node) override;
  virtual void VisitBool(const real_talk::parser::BoolNode &node) override;
  virtual void VisitBreak(const real_talk::parser::BreakNode &node) override;
  virtual void VisitCall(const real_talk::parser::CallNode &node) override;
  virtual void VisitChar(const real_talk::parser::CharNode &node) override;
  virtual void VisitContinue(
      const real_talk::parser::ContinueNode &node) override;
  virtual void VisitDiv(const real_talk::parser::DivNode &node) override;
  virtual void VisitDouble(const real_talk::parser::DoubleNode &node) override;
  virtual void VisitEqual(const real_talk::parser::EqualNode &node) override;
  virtual void VisitExprStmt(
      const real_talk::parser::ExprStmtNode &node) override;
  virtual void VisitFuncDefWithBody(
      const real_talk::parser::FuncDefWithBodyNode &node) override;
  virtual void VisitFuncDefWithoutBody(
      const real_talk::parser::FuncDefWithoutBodyNode &node) override;
  virtual void VisitGreater(
      const real_talk::parser::GreaterNode &node) override;
  virtual void VisitGreaterOrEqual(
      const real_talk::parser::GreaterOrEqualNode &node) override;
  virtual void VisitIfElseIfElse(
      const real_talk::parser::IfElseIfElseNode &node) override;
  virtual void VisitIfElseIf(
      const real_talk::parser::IfElseIfNode &node) override;
  virtual void VisitImport(const real_talk::parser::ImportNode &node) override;
  virtual void VisitInt(const real_talk::parser::IntNode &node) override;
  virtual void VisitLess(const real_talk::parser::LessNode &node) override;
  virtual void VisitLessOrEqual(
      const real_talk::parser::LessOrEqualNode &node) override;
  virtual void VisitLong(const real_talk::parser::LongNode &node) override;
  virtual void VisitMul(const real_talk::parser::MulNode &node) override;
  virtual void VisitNegative(
      const real_talk::parser::NegativeNode &node) override;
  virtual void VisitNotEqual(
      const real_talk::parser::NotEqualNode &node) override;
  virtual void VisitNot(const real_talk::parser::NotNode &node) override;
  virtual void VisitOr(const real_talk::parser::OrNode &node) override;
  virtual void VisitPreDec(const real_talk::parser::PreDecNode &node) override;
  virtual void VisitPreInc(const real_talk::parser::PreIncNode &node) override;
  virtual void VisitPreTestLoop(
      const real_talk::parser::PreTestLoopNode &node) override;
  virtual void VisitProgram(
      const real_talk::parser::ProgramNode &node) override;
  virtual void VisitString(const real_talk::parser::StringNode &node) override;
  virtual void VisitSubscript(
      const real_talk::parser::SubscriptNode &node) override;
  virtual void VisitSub(const real_talk::parser::SubNode &node) override;
  virtual void VisitSum(const real_talk::parser::SumNode &node) override;
  virtual void VisitVarDefWithoutInit(
      const real_talk::parser::VarDefWithoutInitNode &node) override;
  virtual void VisitVarDefWithInit(
      const real_talk::parser::VarDefWithInitNode &node) override;
  virtual void VisitId(
      const real_talk::parser::IdNode &node) override;
  virtual void VisitIntDataType(
      const real_talk::parser::IntDataTypeNode &node) override;
  virtual void VisitLongDataType(
      const real_talk::parser::LongDataTypeNode &node) override;
  virtual void VisitDoubleDataType(
      const real_talk::parser::DoubleDataTypeNode &node) override;
  virtual void VisitCharDataType(
      const real_talk::parser::CharDataTypeNode &node) override;
  virtual void VisitStringDataType(
      const real_talk::parser::StringDataTypeNode &node) override;
  virtual void VisitBoolDataType(
      const real_talk::parser::BoolDataTypeNode &node) override;
  virtual void VisitVoidDataType(
      const real_talk::parser::VoidDataTypeNode &node) override;
  virtual void VisitArrayDataType(
      const real_talk::parser::ArrayDataTypeNode &node) override;
  virtual void VisitReturnValue(
      const real_talk::parser::ReturnValueNode &node) override;
  virtual void VisitReturnWithoutValue(
      const real_talk::parser::ReturnWithoutValueNode &node) override;
  virtual void VisitArgDef(
      const real_talk::parser::ArgDefNode &node) override;

 private:
  class Scope;
  class FileScope;
  class FuncScope;
  class LoopScope;
  class SemanticErrorException;
  class DataTypeQuery;
  class IsVoidDataType;
  class IsFuncDataType;
  class IsArrayDataType;
  class IsDataTypeSupportedByLess;
  class IsDataTypeSupportedByEqual;
  class IsDataTypeSupportedByFuncDef;
  class IsDataTypeSupportedByVarDef;

  typedef std::unordered_map<std::string,
                             const real_talk::parser::DefNode*> IdDefs;
  typedef std::unordered_map<boost::filesystem::path,
                             std::shared_ptr<
                               real_talk::parser::ProgramNode>,
                             boost::hash<
                               boost::filesystem::path> > ImportPrograms;

  void VisitBranch(const real_talk::parser::BranchNode &branch_node);
  std::unique_ptr<DataType> VisitArrayAlloc(
      const real_talk::parser::ArrayAllocNode &array_alloc_node);
  const DataType &VisitVarDef(
      const real_talk::parser::VarDefNode &var_def_node);
  void VisitFuncDef(const real_talk::parser::FuncDefNode &func_def_node,
                    bool &is_func_native,
                    std::unique_ptr<DataType> &return_data_type);
  void VisitReturn(const real_talk::parser::ReturnNode &return_node);
  std::unique_ptr<DataType> CreateDataType(
      const real_talk::parser::DataTypeNode &data_type_node);
  const ExprAnalysis &GetExprAnalysis(const real_talk::parser::ExprNode *expr);
  const DataType &GetExprDataType(const real_talk::parser::ExprNode *expr);
  const DataType &GetFuncReturnDataType(
      const real_talk::parser::FuncDefNode *func_def);
  bool IsCurrentScopeGlobal();
  bool IsWithinImportProgram();
  bool IsTypeConvertible(const DataType &dest, const DataType &src);
  const boost::filesystem::path &GetCurrentFilePath();
  void AddDefAnalyzes(const real_talk::parser::DefNode &def_node,
                      std::unique_ptr<DefAnalysis> def_analysis);
  void AddLitAnalyzes(const real_talk::parser::LitNode *lit_node,
                      std::unique_ptr<DataType> data_type,
                      std::unique_ptr<Lit> lit);

  std::shared_ptr<real_talk::parser::ProgramNode> program_;
  const boost::filesystem::path file_path_;
  const real_talk::parser::FileParser &file_parser_;
  const ImportFileSearcher &import_file_searcher_;
  const LitParser &lit_parser_;
  const DataTypeConverter &data_type_converter_;
  std::vector< std::unique_ptr<SemanticProblem> > problems_;
  SemanticAnalysis::DefAnalyzes def_analyzes_;
  SemanticAnalysis::ExprAnalyzes expr_analyzes_;
  SemanticAnalysis::LitAnalyzes lit_analyzes_;
  SemanticAnalysis::ImportAnalyzes import_analyzes_;
  SemanticAnalysis::IdAnalyzes id_analyzes_;
  ImportPrograms import_programs_;
  std::vector<Scope*> scopes_stack_;
  std::vector<FileScope*> file_scopes_stack_;
  std::vector<FuncScope*> func_scopes_stack_;
  std::vector<LoopScope*> loop_scopes_stack_;
  std::unique_ptr<DataType> current_data_type_;
  size_t non_import_stmts_count_;
};

class SimpleSemanticAnalyzer::Impl::SemanticErrorException
    : public std::runtime_error {
 public:
  explicit SemanticErrorException(unique_ptr<SemanticError> error)
      : runtime_error(""), error_(move(error)) {
  }

  unique_ptr<SemanticError> GetError() {
    return move(error_);
  }

 private:
  std::unique_ptr<SemanticError> error_;
};

class SimpleSemanticAnalyzer::Impl::Scope {
 public:
  explicit Scope(vector<Scope*> &scopes_stack)
      : scopes_stack_(scopes_stack) {
    scopes_stack_.push_back(this);
  }

  ~Scope() {
    scopes_stack_.pop_back();
  }

  const IdDefs &GetIdDefs() const {
    return id_defs_;
  }

  IdDefs &GetIdDefs() {
    return id_defs_;
  }

 private:
  IdDefs id_defs_;
  std::vector<Scope*> &scopes_stack_;
};

class SimpleSemanticAnalyzer::Impl::FileScope {
 public:
  FileScope(
      vector<FileScope*> &file_scopes_stack, const path &file_path)
      : file_scopes_stack_(file_scopes_stack), file_path_(file_path) {
    file_scopes_stack_.push_back(this);
  }

  ~FileScope() {
    file_scopes_stack_.pop_back();
  }

  const path &GetFilePath() const {
    return file_path_;
  }

 private:
  std::vector<FileScope*> &file_scopes_stack_;
  const boost::filesystem::path &file_path_;
};

class SimpleSemanticAnalyzer::Impl::FuncScope {
 public:
  FuncScope(
      vector<FuncScope*> &func_scopes_stack,
      const FuncDefNode &func_def)
      : func_scopes_stack_(func_scopes_stack),
        func_def_(func_def),
        has_return_value_(false) {
    func_scopes_stack_.push_back(this);
  }

  ~FuncScope() {
    func_scopes_stack_.pop_back();
  }

  bool HasReturnValue() const {
    return has_return_value_;
  }

  void SetHasReturnValue(bool has_return_value) {
    has_return_value_ = has_return_value;
  }

  const FuncDefNode &GetFuncDef() const {
    return func_def_;
  }

 private:
  std::vector<FuncScope*> &func_scopes_stack_;
  const FuncDefNode &func_def_;
  bool has_return_value_;
};

class SimpleSemanticAnalyzer::Impl::LoopScope {
 public:
  explicit LoopScope(vector<LoopScope*> &scopes_stack)
      : scopes_stack_(scopes_stack) {
    scopes_stack_.push_back(this);
  }

  ~LoopScope() {
    scopes_stack_.pop_back();
  }

 private:
  std::vector<LoopScope*> &scopes_stack_;
};

class SimpleSemanticAnalyzer::Impl::DataTypeQuery: protected DataTypeVisitor {
 public:
  bool Check(const DataType &data_type) {
    data_type.Accept(*this);
    return result;
  }

 protected:
  DataTypeQuery(): result(false) {}
  virtual ~DataTypeQuery() {}
  virtual void VisitArray(const ArrayDataType&) override {}
  virtual void VisitFunc(const FuncDataType&) override {}
  virtual void VisitBool(const BoolDataType&) override {}
  virtual void VisitInt(const IntDataType&) override {}
  virtual void VisitLong(const LongDataType&) override {}
  virtual void VisitDouble(const DoubleDataType&) override {}
  virtual void VisitChar(const CharDataType&) override {}
  virtual void VisitString(const StringDataType&) override {}
  virtual void VisitVoid(const VoidDataType&) override {}

  bool result;
};

class SimpleSemanticAnalyzer::Impl::IsVoidDataType: public DataTypeQuery {
 protected:
  virtual void VisitArray(const ArrayDataType &data_type) override {
    data_type.GetElementDataType().Accept(*this);
  }

  virtual void VisitVoid(const VoidDataType&) override {
    result = true;
  }
};

class SimpleSemanticAnalyzer::Impl::IsFuncDataType: public DataTypeQuery {
 protected:
  virtual void VisitFunc(const FuncDataType&) override {
    result = true;
  }
};

class SimpleSemanticAnalyzer::Impl::IsArrayDataType: public DataTypeQuery {
 protected:
  virtual void VisitArray(const ArrayDataType&) override {
    result = true;
  }
};

class SimpleSemanticAnalyzer::Impl::IsDataTypeSupportedByFuncDef
    : public DataTypeQuery {
 protected:
  virtual void VisitArray(const ArrayDataType &data_type) override {
    result = !IsVoidDataType().Check(data_type.GetElementDataType());
  }

  virtual void VisitBool(const BoolDataType&) override {result = true;}
  virtual void VisitInt(const IntDataType&) override {result = true;}
  virtual void VisitLong(const LongDataType&) override {result = true;}
  virtual void VisitDouble(const DoubleDataType&) override {result = true;}
  virtual void VisitChar(const CharDataType&) override {result = true;}
  virtual void VisitString(const StringDataType&) override {result = true;}
  virtual void VisitVoid(const VoidDataType&) override {result = true;}
};

class SimpleSemanticAnalyzer::Impl::IsDataTypeSupportedByVarDef
    : public DataTypeQuery {
 protected:
  virtual void VisitArray(const ArrayDataType &data_type) override {
    result = !IsVoidDataType().Check(data_type.GetElementDataType());
  }

  virtual void VisitBool(const BoolDataType&) override {result = true;}
  virtual void VisitInt(const IntDataType&) override {result = true;}
  virtual void VisitLong(const LongDataType&) override {result = true;}
  virtual void VisitDouble(const DoubleDataType&) override {result = true;}
  virtual void VisitChar(const CharDataType&) override {result = true;}
  virtual void VisitString(const StringDataType&) override {result = true;}
};

class SimpleSemanticAnalyzer::Impl::IsDataTypeSupportedByLess
    : public DataTypeQuery {
 protected:
  virtual void VisitInt(const IntDataType&) override {result = true;}
  virtual void VisitLong(const LongDataType&) override {result = true;}
  virtual void VisitDouble(const DoubleDataType&) override {result = true;}
};

class SimpleSemanticAnalyzer::Impl::IsDataTypeSupportedByEqual
    : public DataTypeQuery {
 protected:
  virtual void VisitArray(const ArrayDataType &data_type) override {
    result = IsVoidDataType().Check(data_type);
  }

  virtual void VisitBool(const BoolDataType&) override {result = true;}
  virtual void VisitInt(const IntDataType&) override {result = true;}
  virtual void VisitLong(const LongDataType&) override {result = true;}
  virtual void VisitDouble(const DoubleDataType&) override {result = true;}
  virtual void VisitChar(const CharDataType&) override {result = true;}
  virtual void VisitString(const StringDataType&) override {result = true;}
};

SimpleSemanticAnalyzer::SimpleSemanticAnalyzer(
    shared_ptr<ProgramNode> program,
    const path &file_path,
    const FileParser &file_parser,
    const ImportFileSearcher &import_file_searcher,
    const LitParser &lit_parser,
    const DataTypeConverter &data_type_converter)
    : impl_(new SimpleSemanticAnalyzer::Impl(program,
                                             file_path,
                                             file_parser,
                                             import_file_searcher,
                                             lit_parser,
                                             data_type_converter)) {
}

SimpleSemanticAnalyzer::~SimpleSemanticAnalyzer() {
}

SemanticAnalysis SimpleSemanticAnalyzer::Analyze() {
  return impl_->Analyze();
}

SimpleSemanticAnalyzer::Impl::Impl(
    shared_ptr<ProgramNode> program,
    const path &file_path,
    const FileParser &file_parser,
    const ImportFileSearcher &import_file_searcher,
    const LitParser &lit_parser,
    const DataTypeConverter &data_type_converter)
    : program_(program),
      file_path_(file_path),
      file_parser_(file_parser),
      import_file_searcher_(import_file_searcher),
      lit_parser_(lit_parser),
      data_type_converter_(data_type_converter),
      non_import_stmts_count_(0) {
}

SemanticAnalysis SimpleSemanticAnalyzer::Impl::Analyze() {
  import_programs_.insert(make_pair(file_path_, program_));

  {
    Scope scope(scopes_stack_);
    FileScope file_scope(file_scopes_stack_, file_path_);
    program_->Accept(*this);
  }

  assert(scopes_stack_.empty());
  assert(file_scopes_stack_.empty());
  assert(func_scopes_stack_.empty());
  assert(loop_scopes_stack_.empty());
  return SemanticAnalysis(move(problems_),
                          move(def_analyzes_),
                          move(expr_analyzes_),
                          move(lit_analyzes_),
                          move(import_analyzes_),
                          move(id_analyzes_));
}

void SimpleSemanticAnalyzer::Impl::VisitProgram(const ProgramNode &program) {
  for (const unique_ptr<StmtNode> &stmt: program.GetStmts()) {
    try {
      stmt->Accept(*this);
    } catch (SemanticErrorException &e) {
      problems_.push_back(e.GetError());
    }
  }
}

void SimpleSemanticAnalyzer::Impl::VisitReturnValue(
    const ReturnValueNode &return_node) {
  if (IsWithinImportProgram()) {
    return;
  }

  VisitReturn(return_node);
  return_node.GetValue()->Accept(*this);
  FuncScope &current_func_scope = *(func_scopes_stack_.back());
  current_func_scope.SetHasReturnValue(true);
  const DataType &func_def_return_data_type =
      GetFuncReturnDataType(&current_func_scope.GetFuncDef());
  const DataType &value_data_type =
      GetExprDataType(return_node.GetValue().get());

  if (!IsTypeConvertible(func_def_return_data_type, value_data_type)) {
    unique_ptr<SemanticError> error(new ReturnWithIncompatibleTypeError(
        GetCurrentFilePath(),
        return_node,
        func_def_return_data_type.Clone(),
        value_data_type.Clone()));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitReturnWithoutValue(
    const ReturnWithoutValueNode &return_node) {
  if (IsWithinImportProgram()) {
    return;
  }

  VisitReturn(return_node);
  FuncScope &current_func_scope = *(func_scopes_stack_.back());
  const DataType &func_def_return_data_type =
      GetFuncReturnDataType(&current_func_scope.GetFuncDef());

  if (func_def_return_data_type != VoidDataType()) {
    unique_ptr<SemanticError> error(
        new ReturnWithoutValueError(GetCurrentFilePath(), return_node));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitReturn(const ReturnNode &return_node) {
  ++non_import_stmts_count_;

  if (func_scopes_stack_.empty()) {
    unique_ptr<SemanticError> error(
        new ReturnNotWithinFuncError(GetCurrentFilePath(), return_node));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitBreak(const BreakNode &break_node) {
  if (IsWithinImportProgram()) {
    return;
  }

  ++non_import_stmts_count_;

  if (loop_scopes_stack_.empty()) {
    unique_ptr<SemanticError> error(
        new BreakNotWithinLoopError(GetCurrentFilePath(), break_node));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitContinue(
    const ContinueNode &continue_node) {
  if (IsWithinImportProgram()) {
    return;
  }

  ++non_import_stmts_count_;

  if (loop_scopes_stack_.empty()) {
    unique_ptr<SemanticError> error(
        new ContinueNotWithinLoopError(GetCurrentFilePath(), continue_node));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitExprStmt(const ExprStmtNode& stmt) {
  if (IsWithinImportProgram()) {
    return;
  }

  ++non_import_stmts_count_;
  stmt.GetExpr()->Accept(*this);
}

void SimpleSemanticAnalyzer::Impl::VisitVarDefWithoutInit(
    const VarDefWithoutInitNode &var_def_node) {
  VisitVarDef(var_def_node);
}

void SimpleSemanticAnalyzer::Impl::VisitVarDefWithInit(
    const VarDefWithInitNode &var_def_node) {
  const DataType &var_data_type = VisitVarDef(var_def_node);

  if (IsWithinImportProgram()) {
    return;
  }

  var_def_node.GetValue()->Accept(*this);
  const DataType &value_data_type =
      GetExprDataType(var_def_node.GetValue().get());

  if (!IsTypeConvertible(var_data_type, value_data_type)) {
    unique_ptr<SemanticError> error(new InitWithIncompatibleTypeError(
        GetCurrentFilePath(),
        var_def_node,
        var_data_type.Clone(),
        value_data_type.Clone()));
    throw SemanticErrorException(move(error));
  }
}

const DataType &SimpleSemanticAnalyzer::Impl::VisitVarDef(
    const VarDefNode &var_def_node) {
  if (!IsWithinImportProgram()) {
    ++non_import_stmts_count_;
  }

  unique_ptr<DataType> data_type_ptr =
      CreateDataType(*(var_def_node.GetDataType()));
  const DataType &data_type = *data_type_ptr;
  DataStorage data_storage =
      IsCurrentScopeGlobal() ? DataStorage::kGlobal : DataStorage::kLocal;
  unique_ptr<DefAnalysis> def_analysis(
      new VarDefAnalysis(move(data_type_ptr), data_storage));
  AddDefAnalyzes(var_def_node, move(def_analysis));

  if (IsWithinImportProgram()) {
    return data_type;
  }

  if (!IsDataTypeSupportedByVarDef().Check(data_type)) {
    unique_ptr<SemanticError> error(new DefWithUnsupportedTypeError(
        GetCurrentFilePath(), var_def_node, data_type.Clone()));
    throw SemanticErrorException(move(error));
  }

  return data_type;
}

void SimpleSemanticAnalyzer::Impl::VisitArgDef(const ArgDefNode &arg_def_node) {
  unique_ptr<DataType> data_type_ptr =
      CreateDataType(*(arg_def_node.GetDataType()));
  const DataType &data_type = *data_type_ptr;
  unique_ptr<DefAnalysis> def_analysis(new ArgDefAnalysis(move(data_type_ptr)));
  AddDefAnalyzes(arg_def_node, move(def_analysis));

  if (!IsDataTypeSupportedByVarDef().Check(data_type)) {
    unique_ptr<SemanticError> error(new DefWithUnsupportedTypeError(
        GetCurrentFilePath(), arg_def_node, data_type.Clone()));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitFuncDefWithBody(
    const FuncDefWithBodyNode &func_def_node) {
  bool is_func_native;
  unique_ptr<DataType> return_data_type;
  VisitFuncDef(func_def_node, is_func_native, return_data_type);

  if (IsWithinImportProgram()) {
    return;
  }

  if (is_func_native) {
    unique_ptr<SemanticError> error(new FuncDefWithBodyIsNativeError(
        GetCurrentFilePath(), func_def_node));
    throw SemanticErrorException(move(error));
  }

  FuncScope func_scope(func_scopes_stack_, func_def_node);
  Scope scope(scopes_stack_);

  for (const unique_ptr<ArgDefNode> &arg_def_node: func_def_node.GetArgs()) {
    arg_def_node->Accept(*this);
  }

  for (const unique_ptr<StmtNode> &stmt: func_def_node.GetBody()->GetStmts()) {
    stmt->Accept(*this);
  }

  if (*return_data_type != VoidDataType() && !func_scope.HasReturnValue()) {
    unique_ptr<SemanticError> error(new FuncDefWithoutReturnValueError(
        GetCurrentFilePath(), func_def_node));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitFuncDefWithoutBody(
    const FuncDefWithoutBodyNode &func_def_node) {
  bool is_func_native;
  unique_ptr<DataType> return_data_type;
  VisitFuncDef(func_def_node, is_func_native, return_data_type);

  if (IsWithinImportProgram()) {
    return;
  }

  if (!is_func_native) {
    unique_ptr<SemanticError> error(new FuncDefWithoutBodyNotNativeError(
        GetCurrentFilePath(), func_def_node));
    throw SemanticErrorException(move(error));
  }

  FuncScope func_scope(func_scopes_stack_, func_def_node);
  Scope scope(scopes_stack_);

  for (const unique_ptr<ArgDefNode> &arg_def_node: func_def_node.GetArgs()) {
    arg_def_node->Accept(*this);
  }
}

void SimpleSemanticAnalyzer::Impl::VisitFuncDef(
    const FuncDefNode &func_def_node,
    bool &is_func_native,
    unique_ptr<DataType> &return_data_type) {
  if (!IsWithinImportProgram()) {
    ++non_import_stmts_count_;
  }

  return_data_type = CreateDataType(*(func_def_node.GetDataType()));
  vector< unique_ptr<DataType> > arg_data_types;

  for (const unique_ptr<ArgDefNode> &arg_def_node: func_def_node.GetArgs()) {
    unique_ptr<DataType> arg_data_type =
        CreateDataType(*(arg_def_node->GetDataType()));
    arg_data_types.push_back(move(arg_data_type));
  }

  unique_ptr<FuncDataType> func_data_type(
      new FuncDataType(return_data_type->Clone(), move(arg_data_types)));
  unordered_set<Token> modifier_tokens;

  for (const TokenInfo &modifier_token: func_def_node.GetModifierTokens()) {
    const bool is_duplicate_modifier =
        !modifier_tokens.insert(modifier_token.GetId()).second;
    assert(!is_duplicate_modifier);
  }

  is_func_native = modifier_tokens.count(Token::kNative) != 0;
  unique_ptr<DefAnalysis> def_analysis(
      new FuncDefAnalysis(move(func_data_type), is_func_native));
  AddDefAnalyzes(func_def_node, move(def_analysis));

  if (IsWithinImportProgram()) {
    return;
  }

  if (!IsCurrentScopeGlobal()) {
    unique_ptr<SemanticError> error(
        new FuncDefWithinNonGlobalScope(GetCurrentFilePath(), func_def_node));
    throw SemanticErrorException(move(error));
  }

  const bool is_return_data_type_supported =
      IsDataTypeSupportedByFuncDef().Check(*return_data_type);

  if (!is_return_data_type_supported) {
    unique_ptr<SemanticError> error(new DefWithUnsupportedTypeError(
        GetCurrentFilePath(), func_def_node, return_data_type->Clone()));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitPreTestLoop(
    const PreTestLoopNode &loop_node) {
  if (IsWithinImportProgram()) {
    return;
  }

  ++non_import_stmts_count_;
  loop_node.GetCond()->Accept(*this);
  const DataType &cond_data_type = GetExprDataType(loop_node.GetCond().get());
  unique_ptr<DataType> bool_data_type(new BoolDataType());

  if (!IsTypeConvertible(*bool_data_type, cond_data_type)) {
    unique_ptr<SemanticError> error(new PreTestLoopWithIncompatibleTypeError(
        GetCurrentFilePath(),
        loop_node,
        move(bool_data_type),
        cond_data_type.Clone()));
    throw SemanticErrorException(move(error));
  }

  LoopScope loop_scope(loop_scopes_stack_);
  Scope scope(scopes_stack_);

  for (const unique_ptr<StmtNode> &stmt: loop_node.GetBody()->GetStmts()) {
    stmt->Accept(*this);
  }
}

void SimpleSemanticAnalyzer::Impl::VisitIfElseIf(
    const IfElseIfNode &if_else_if_node) {
  if (IsWithinImportProgram()) {
    return;
  }

  VisitBranch(if_else_if_node);
}

void SimpleSemanticAnalyzer::Impl::VisitIfElseIfElse(
    const IfElseIfElseNode &if_else_if_else_node) {
  if (IsWithinImportProgram()) {
    return;
  }

  VisitBranch(if_else_if_else_node);
  const vector< unique_ptr<StmtNode> > &else_stmts =
      if_else_if_else_node.GetElseBody()->GetStmts();
  Scope scope(scopes_stack_);

  for (const unique_ptr<StmtNode> &else_stmt: else_stmts) {
    else_stmt->Accept(*this);
  }
}

void SimpleSemanticAnalyzer::Impl::VisitBranch(const BranchNode &branch_node) {
  ++non_import_stmts_count_;
  const ExprNode &if_cond = *(branch_node.GetIf()->GetCond());
  if_cond.Accept(*this);

  {
    unique_ptr<DataType> bool_data_type(new BoolDataType());
    const DataType &if_cond_data_type = GetExprDataType(&if_cond);

    if (!IsTypeConvertible(*bool_data_type, if_cond_data_type)) {
      unique_ptr<SemanticError> error(new IfWithIncompatibleTypeError(
          GetCurrentFilePath(),
          branch_node,
          *(branch_node.GetIf()),
          move(bool_data_type),
          if_cond_data_type.Clone()));
      throw SemanticErrorException(move(error));
    }
  }

  const vector< unique_ptr<StmtNode> > &if_stmts =
      branch_node.GetIf()->GetBody()->GetStmts();

  {
    Scope scope(scopes_stack_);

    for (const unique_ptr<StmtNode> &stmt: if_stmts) {
      stmt->Accept(*this);
    }
  }

  for (const unique_ptr<ElseIfNode> &else_if: branch_node.GetElseIfs()) {
    const ExprNode &else_if_cond = *(else_if->GetIf()->GetCond());
    else_if_cond.Accept(*this);
    const DataType &else_if_cond_data_type = GetExprDataType(&else_if_cond);
    unique_ptr<DataType> bool_data_type(new BoolDataType);

    if (!IsTypeConvertible(*bool_data_type, else_if_cond_data_type)) {
      unique_ptr<SemanticError> error(new IfWithIncompatibleTypeError(
          GetCurrentFilePath(),
          branch_node,
          *(else_if->GetIf()),
          move(bool_data_type),
          else_if_cond_data_type.Clone()));
      throw SemanticErrorException(move(error));
    }

    const vector< unique_ptr<StmtNode> > &else_if_stmts =
        else_if->GetIf()->GetBody()->GetStmts();
    Scope scope(scopes_stack_);

    for (const unique_ptr<StmtNode> &stmt: else_if_stmts) {
      stmt->Accept(*this);
    }
  }
}

void SimpleSemanticAnalyzer::Impl::VisitImport(const ImportNode &import_node) {
  import_node.GetFilePath()->Accept(*this);
  SemanticAnalysis::LitAnalyzes::const_iterator lit_analysis_it =
      lit_analyzes_.find(import_node.GetFilePath().get());
  assert(lit_analysis_it != lit_analyzes_.end());
  const LitAnalysis &lit_analysis = lit_analysis_it->second;
  const StringLit &lit = static_cast<const StringLit&>(lit_analysis.GetLit());
  const path relative_file_path(lit.GetValue());
  path absolute_file_path;

  try {
    absolute_file_path = import_file_searcher_.Search(relative_file_path);
  } catch (const FileNotFoundError&) {
    unique_ptr<SemanticError> error(new ImportWithNotExistingFileError(
        GetCurrentFilePath(), import_node, relative_file_path));
    throw SemanticErrorException(move(error));
  } catch (const IOError&) {
    unique_ptr<SemanticError> error(new ImportWithIOError(
        GetCurrentFilePath(), import_node, relative_file_path));
    throw SemanticErrorException(move(error));
  }

  ImportPrograms::const_iterator import_program_it =
      import_programs_.find(absolute_file_path);
  const bool is_file_already_analyzed =
      import_program_it != import_programs_.end();
  shared_ptr<ProgramNode> program;

  if (is_file_already_analyzed) {
    program = import_program_it->second;
  } else {
    try {
      program = file_parser_.Parse(absolute_file_path);
    } catch (const IOError&) {
      unique_ptr<SemanticError> error(new ImportWithIOError(
          GetCurrentFilePath(), import_node, absolute_file_path));
      throw SemanticErrorException(move(error));
    } catch (const UnexpectedTokenError &e) {
      unique_ptr<SemanticError> error(new ImportWithUnexpectedTokenError(
          GetCurrentFilePath(), import_node, absolute_file_path, e.GetToken()));
      throw SemanticErrorException(move(error));
    } catch (const UnexpectedCharError &e) {
      unique_ptr<SemanticError> error(new ImportWithUnexpectedCharError(
          GetCurrentFilePath(),
          import_node,
          absolute_file_path,
          e.GetChar(),
          e.GetLineNumber(),
          e.GetColumnNumber()));
      throw SemanticErrorException(move(error));
    }

    import_programs_.insert(make_pair(absolute_file_path, program));
    FileScope file_scope(file_scopes_stack_, absolute_file_path);
    program->Accept(*this);
  }

  const ImportAnalysis import_analysis(program);
  import_analyzes_.insert(make_pair(&import_node, import_analysis));

  if (non_import_stmts_count_ != 0) {
    unique_ptr<SemanticError> error(
        new ImportIsNotFirstStmtError(GetCurrentFilePath(), import_node));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitAnd(const AndNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitArrayAllocWithoutInit(
    const ArrayAllocWithoutInitNode &alloc_node) {
  unique_ptr<DataType> element_data_type = VisitArrayAlloc(alloc_node);
  unique_ptr<DataType> &array_data_type = element_data_type;
  assert(!alloc_node.GetSizes().empty());

  for (const unique_ptr<BoundedArraySizeNode> &size: alloc_node.GetSizes()) {
    array_data_type.reset(new ArrayDataType(move(array_data_type)));
    const ExprNode &size_value = *(size->GetValue());
    size_value.Accept(*this);
    const DataType &size_data_type = GetExprDataType(&size_value);
    unique_ptr<DataType> int_data_type(new IntDataType());

    if (!IsTypeConvertible(*int_data_type, size_data_type)) {
      unique_ptr<SemanticError> error(
          new ArrayAllocWithIncompatibleSizeTypeError(
              GetCurrentFilePath(),
              alloc_node,
              *size,
              move(int_data_type),
              size_data_type.Clone()));
      throw SemanticErrorException(move(error));
    }
  }

  ExprAnalysis expr_analysis(move(array_data_type), ValueType::kRight);
  expr_analyzes_.insert(make_pair(&alloc_node, move(expr_analysis)));
}

void SimpleSemanticAnalyzer::Impl::VisitArrayAllocWithInit(
    const ArrayAllocWithInitNode &alloc_node) {
  unique_ptr<DataType> array_data_type = VisitArrayAlloc(alloc_node);
  assert(!alloc_node.GetSizes().empty());

  for (const unique_ptr<UnboundedArraySizeNode> &size: alloc_node.GetSizes()) {
    array_data_type.reset(new ArrayDataType(move(array_data_type)));
    (void) size;
  }

  const vector< unique_ptr<ExprNode> > &values = alloc_node.GetValues();
  auto value_end_it = values.end();

  for (auto value_it = values.begin();
       value_it != value_end_it;
       ++value_it) {
    const ExprNode &value = **value_it;
    value.Accept(*this);
    const DataType &value_data_type = GetExprDataType(&value);
    const DataType &element_data_type = static_cast<const ArrayDataType&>(
        *array_data_type).GetElementDataType();

    if (!IsTypeConvertible(element_data_type, value_data_type)) {
      size_t value_index = static_cast<size_t>(value_it - values.begin());
      unique_ptr<SemanticError> error(
          new ArrayAllocWithIncompatibleValueTypeError(
              GetCurrentFilePath(),
              alloc_node,
              value_index,
              element_data_type.Clone(),
              value_data_type.Clone()));
      throw SemanticErrorException(move(error));
    }
  }

  ExprAnalysis expr_analysis(move(array_data_type), ValueType::kRight);
  expr_analyzes_.insert(make_pair(&alloc_node, move(expr_analysis)));
}

unique_ptr<DataType> SimpleSemanticAnalyzer::Impl::VisitArrayAlloc(
    const ArrayAllocNode &alloc_node) {
  unique_ptr<DataType> element_data_type =
      CreateDataType(*(alloc_node.GetDataType()));

  if (*element_data_type == VoidDataType()) {
    unique_ptr<SemanticError> error(
        new ArrayAllocWithUnsupportedElementTypeError(
            GetCurrentFilePath(), alloc_node, element_data_type->Clone()));
    throw SemanticErrorException(move(error));
  }

  return element_data_type;
}

void SimpleSemanticAnalyzer::Impl::VisitAssign(const AssignNode &assign_node) {
  assign_node.GetLeftOperand()->Accept(*this);
  const ExprAnalysis &left_operand_expr_analysis =
      GetExprAnalysis(assign_node.GetLeftOperand().get());

  if (left_operand_expr_analysis.GetValueType() == ValueType::kRight) {
    unique_ptr<SemanticError> error(new AssignWithRightValueAssigneeError(
        GetCurrentFilePath(), assign_node));
    throw SemanticErrorException(move(error));
  }

  assign_node.GetRightOperand()->Accept(*this);
  const DataType &left_operand_data_type =
      left_operand_expr_analysis.GetDataType();
  const DataType &right_operand_data_type =
      GetExprDataType(assign_node.GetRightOperand().get());

  if (!IsTypeConvertible(left_operand_data_type, right_operand_data_type)) {
    unique_ptr<SemanticError> error(new BinaryExprWithIncompatibleTypeError(
        GetCurrentFilePath(),
        assign_node,
        left_operand_data_type.Clone(),
        right_operand_data_type.Clone()));
    throw SemanticErrorException(move(error));
  }

  ExprAnalysis assign_analysis(left_operand_data_type.Clone(),
                               ValueType::kLeft);
  expr_analyzes_.insert(make_pair(&assign_node, move(assign_analysis)));
}

void SimpleSemanticAnalyzer::Impl::VisitDiv(const DivNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitCall(const CallNode &call_node) {
  call_node.GetOperand()->Accept(*this);
  const DataType &operand_data_type =
      GetExprDataType(call_node.GetOperand().get());

  if (!IsFuncDataType().Check(operand_data_type)) {
    unique_ptr<SemanticError> error(
        new CallWithNonFuncError(GetCurrentFilePath(), call_node));
    throw SemanticErrorException(move(error));
  }

  const FuncDataType &func_data_type =
      static_cast<const FuncDataType&>(operand_data_type);
  ExprAnalysis expr_analysis(func_data_type.GetReturnDataType().Clone(),
                             ValueType::kRight);
  expr_analyzes_.insert(make_pair(&call_node, move(expr_analysis)));
  size_t expected_args_count = func_data_type.GetArgDataTypes().size();
  size_t actual_args_count = call_node.GetArgs().size();

  if (expected_args_count != actual_args_count) {
    unique_ptr<SemanticError> error(new CallWithInvalidArgsCount(
        GetCurrentFilePath(),
        call_node,
        expected_args_count,
        actual_args_count));
    throw SemanticErrorException(move(error));
  }

  assert(expected_args_count == actual_args_count);
  vector< unique_ptr<ExprNode> >::const_iterator call_arg_it =
      call_node.GetArgs().begin();

  for (const unique_ptr<DataType> &arg_def_data_type_ptr:
           func_data_type.GetArgDataTypes()) {
    const ExprNode &call_arg = **call_arg_it;
    call_arg.Accept(*this);
    const DataType &call_arg_data_type = GetExprDataType(&call_arg);
    const DataType &arg_def_data_type = *arg_def_data_type_ptr;

    if (!IsTypeConvertible(arg_def_data_type, call_arg_data_type)) {
      const size_t arg_index =
          static_cast<size_t>(call_arg_it - call_node.GetArgs().begin());
      unique_ptr<SemanticError> error(new CallWithIncompatibleTypeError(
          GetCurrentFilePath(),
          call_node,
          arg_index,
          arg_def_data_type.Clone(),
          call_arg_data_type.Clone()));
      throw SemanticErrorException(move(error));
    }

    ++call_arg_it;
  }
}

void SimpleSemanticAnalyzer::Impl::VisitGreater(const GreaterNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitGreaterOrEqual(
    const GreaterOrEqualNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitBool(const BoolNode &bool_node) {
  const bool value = lit_parser_.ParseBool(bool_node.GetToken().GetValue());
  unique_ptr<DataType> data_type(new BoolDataType());
  unique_ptr<Lit> lit(new BoolLit(value));
  AddLitAnalyzes(&bool_node, move(data_type), move(lit));
}

void SimpleSemanticAnalyzer::Impl::VisitInt(const IntNode &int_node) {
  int32_t value;

  try {
    value = lit_parser_.ParseInt(int_node.GetToken().GetValue());
  } catch (...) {
    // TODO
    assert(false);
  }

  unique_ptr<DataType> data_type(new IntDataType());
  unique_ptr<Lit> lit(new IntLit(value));
  AddLitAnalyzes(&int_node, move(data_type), move(lit));
}

void SimpleSemanticAnalyzer::Impl::VisitLong(const LongNode &long_node) {
  int64_t value;

  try {
    value = lit_parser_.ParseLong(long_node.GetToken().GetValue());
  } catch (...) {
    // TODO
    assert(false);
  }

  unique_ptr<DataType> data_type(new LongDataType());
  unique_ptr<Lit> lit(new LongLit(value));
  AddLitAnalyzes(&long_node, move(data_type), move(lit));
}

void SimpleSemanticAnalyzer::Impl::VisitDouble(const DoubleNode &double_node) {
  double value;

  try {
    value = lit_parser_.ParseDouble(double_node.GetToken().GetValue());
  } catch (...) {
    // TODO
    assert(false);
  }

  unique_ptr<DataType> data_type(new DoubleDataType());
  unique_ptr<Lit> lit(new DoubleLit(value));
  AddLitAnalyzes(&double_node, move(data_type), move(lit));
}

void SimpleSemanticAnalyzer::Impl::VisitChar(const CharNode &char_node) {
  char value;

  try {
    value = lit_parser_.ParseChar(char_node.GetToken().GetValue());
  } catch (...) {
    // TODO
    assert(false);
  }

  unique_ptr<DataType> data_type(new CharDataType());
  unique_ptr<Lit> lit(new CharLit(value));
  AddLitAnalyzes(&char_node, move(data_type), move(lit));
}

void SimpleSemanticAnalyzer::Impl::VisitString(const StringNode &string_node) {
  string value;

  try {
    value = lit_parser_.ParseString(string_node.GetToken().GetValue());
  } catch (...) {
    // TODO
    assert(false);
  }

  unique_ptr<DataType> data_type(new StringDataType());
  unique_ptr<Lit> lit(new StringLit(value));
  AddLitAnalyzes(&string_node, move(data_type), move(lit));
}

void SimpleSemanticAnalyzer::Impl::VisitEqual(const EqualNode &equal_node) {
  equal_node.GetLeftOperand()->Accept(*this);
  equal_node.GetRightOperand()->Accept(*this);
  const DataType &left_operand_data_type =
      GetExprDataType(equal_node.GetLeftOperand().get());
  const DataType &right_operand_data_type =
      GetExprDataType(equal_node.GetRightOperand().get());

  if (!IsTypeConvertible(left_operand_data_type, right_operand_data_type)) {
    unique_ptr<SemanticError> error(new BinaryExprWithIncompatibleTypeError(
        GetCurrentFilePath(),
        equal_node,
        left_operand_data_type.Clone(),
        right_operand_data_type.Clone()));
    throw SemanticErrorException(move(error));
  }

  if (!IsDataTypeSupportedByEqual().Check(left_operand_data_type)) {
    unique_ptr<SemanticError> error(new BinaryExprWithUnsupportedTypesError(
        GetCurrentFilePath(),
        equal_node,
        left_operand_data_type.Clone(),
        right_operand_data_type.Clone()));
    throw SemanticErrorException(move(error));
  }

  unique_ptr<DataType> equal_data_type(new BoolDataType());
  ExprAnalysis equal_analysis(move(equal_data_type), ValueType::kRight);
  expr_analyzes_.insert(make_pair(&equal_node, move(equal_analysis)));
}

void SimpleSemanticAnalyzer::Impl::VisitLess(const LessNode &less_node) {
  less_node.GetLeftOperand()->Accept(*this);
  less_node.GetRightOperand()->Accept(*this);
  unique_ptr<DataType> less_data_type(new BoolDataType());
  ExprAnalysis less_analysis(move(less_data_type), ValueType::kRight);
  expr_analyzes_.insert(make_pair(&less_node, move(less_analysis)));
  const DataType &left_operand_data_type =
      GetExprDataType(less_node.GetLeftOperand().get());
  const DataType &right_operand_data_type =
      GetExprDataType(less_node.GetRightOperand().get());

  if (!IsTypeConvertible(left_operand_data_type, right_operand_data_type)) {
    unique_ptr<SemanticError> error(new BinaryExprWithIncompatibleTypeError(
        GetCurrentFilePath(),
        less_node,
        left_operand_data_type.Clone(),
        right_operand_data_type.Clone()));
    throw SemanticErrorException(move(error));
  }

  if (!IsDataTypeSupportedByLess().Check(left_operand_data_type)) {
    unique_ptr<SemanticError> error(new BinaryExprWithUnsupportedTypesError(
        GetCurrentFilePath(),
        less_node,
        left_operand_data_type.Clone(),
        right_operand_data_type.Clone()));
    throw SemanticErrorException(move(error));
  }
}

void SimpleSemanticAnalyzer::Impl::VisitLessOrEqual(const LessOrEqualNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitMul(const MulNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitNegative(const NegativeNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitNotEqual(const NotEqualNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitNot(const NotNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitOr(const OrNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitPreDec(const PreDecNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitPreInc(const PreIncNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitSubscript(
    const SubscriptNode &subscript) {
  subscript.GetOperand()->Accept(*this);
  const ExprAnalysis &operand_expr_analysis =
      GetExprAnalysis(subscript.GetOperand().get());
  const DataType &operand_data_type = operand_expr_analysis.GetDataType();

  if (!IsArrayDataType().Check(operand_data_type)) {
    unique_ptr<SemanticError> error(
        new SubscriptWithNonArrayError(GetCurrentFilePath(), subscript));
    throw SemanticErrorException(move(error));
  }

  subscript.GetIndex()->Accept(*this);
  const DataType &index_data_type = GetExprDataType(subscript.GetIndex().get());
  unique_ptr<DataType> int_data_type(new IntDataType());

  if (!IsTypeConvertible(*int_data_type, index_data_type)) {
    unique_ptr<SemanticError> error(new SubscriptWithIncompatibleIndexTypeError(
        GetCurrentFilePath(),
        subscript,
        move(int_data_type),
        index_data_type.Clone()));
    throw SemanticErrorException(move(error));
  }

  const ArrayDataType &array_data_type =
      static_cast<const ArrayDataType&>(operand_data_type);
  const ValueType operand_value_type = operand_expr_analysis.GetValueType();
  ExprAnalysis expr_analysis(array_data_type.GetElementDataType().Clone(),
                             operand_value_type);
  expr_analyzes_.insert(make_pair(&subscript, move(expr_analysis)));
}

void SimpleSemanticAnalyzer::Impl::VisitSub(const SubNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitSum(const SumNode&) {
}

void SimpleSemanticAnalyzer::Impl::VisitId(const IdNode &id_node) {
  const DefNode *def_node = nullptr;
  const string &id = id_node.GetNameToken().GetValue();

  for (const Scope* scope: reverse(scopes_stack_)) {
    const IdDefs &scope_id_defs = scope->GetIdDefs();
    IdDefs::const_iterator scope_id_def_it = scope_id_defs.find(id);

    if (scope_id_def_it != scope_id_defs.end()) {
      def_node = scope_id_def_it->second;
      break;
    }
  }

  if (def_node == nullptr) {
    unique_ptr<SemanticError> error(
        new IdWithoutDefError(GetCurrentFilePath(), id_node));
    throw SemanticErrorException(move(error));
  }

  SemanticAnalysis::DefAnalyzes::const_iterator def_analysis_it =
      def_analyzes_.find(def_node);
  assert(def_analysis_it != def_analyzes_.end());
  const DefAnalysis &def_analysis = *(def_analysis_it->second);
  ExprAnalysis expr_analysis(def_analysis.GetDataType().Clone(),
                             def_analysis.GetValueType());
  expr_analyzes_.insert(make_pair(&id_node, move(expr_analysis)));
  const IdAnalysis id_analysis(def_node);
  id_analyzes_.insert(make_pair(&id_node, id_analysis));
}

void SimpleSemanticAnalyzer::Impl::VisitIntDataType(
    const IntDataTypeNode&) {
  current_data_type_.reset(new IntDataType());
}

void SimpleSemanticAnalyzer::Impl::VisitLongDataType(
    const LongDataTypeNode&) {
  current_data_type_.reset(new LongDataType());
}

void SimpleSemanticAnalyzer::Impl::VisitDoubleDataType(
    const DoubleDataTypeNode&) {
  current_data_type_.reset(new DoubleDataType());
}

void SimpleSemanticAnalyzer::Impl::VisitCharDataType(
    const CharDataTypeNode&) {
  current_data_type_.reset(new CharDataType());
}

void SimpleSemanticAnalyzer::Impl::VisitStringDataType(
    const StringDataTypeNode&) {
  current_data_type_.reset(new StringDataType());
}

void SimpleSemanticAnalyzer::Impl::VisitBoolDataType(
    const BoolDataTypeNode&) {
  current_data_type_.reset(new BoolDataType());
}

void SimpleSemanticAnalyzer::Impl::VisitVoidDataType(
    const VoidDataTypeNode&) {
  current_data_type_.reset(new VoidDataType());
}

void SimpleSemanticAnalyzer::Impl::VisitArrayDataType(
    const ArrayDataTypeNode &data_type_node) {
  unique_ptr<DataType> element_data_type =
      CreateDataType(*(data_type_node.GetElementDataType()));
  current_data_type_.reset(new ArrayDataType(
      move(element_data_type)));
}

unique_ptr<DataType> SimpleSemanticAnalyzer::Impl::CreateDataType(
    const DataTypeNode &data_type_node) {
  assert(!current_data_type_);
  data_type_node.Accept(*this);
  assert(current_data_type_);
  return move(current_data_type_);
}

const ExprAnalysis &SimpleSemanticAnalyzer::Impl::GetExprAnalysis(
    const ExprNode *expr) {
  SemanticAnalysis::ExprAnalyzes::const_iterator expr_analysis_it =
      expr_analyzes_.find(expr);
  assert(expr_analysis_it != expr_analyzes_.end());
  return expr_analysis_it->second;
}

const DataType &SimpleSemanticAnalyzer::Impl::GetExprDataType(
    const ExprNode *expr) {
  return GetExprAnalysis(expr).GetDataType();
}

const DataType &SimpleSemanticAnalyzer::Impl::GetFuncReturnDataType(
    const FuncDefNode *func_def) {
  SemanticAnalysis::DefAnalyzes::const_iterator func_def_analysis_it =
      def_analyzes_.find(func_def);
  assert(func_def_analysis_it != def_analyzes_.end());
  const FuncDefAnalysis &func_def_analysis =
      static_cast<const FuncDefAnalysis&>(*(func_def_analysis_it->second));
  return func_def_analysis.GetDataType().GetReturnDataType();
}

void SimpleSemanticAnalyzer::Impl::AddLitAnalyzes(
    const LitNode *lit_node,
    unique_ptr<DataType> data_type,
    unique_ptr<Lit> lit) {
  ExprAnalysis expr_analysis(move(data_type), ValueType::kRight);
  expr_analyzes_.insert(make_pair(lit_node, move(expr_analysis)));
  LitAnalysis lit_analysis(move(lit));
  lit_analyzes_.insert(make_pair(lit_node, move(lit_analysis)));
}

void SimpleSemanticAnalyzer::Impl::AddDefAnalyzes(
    const DefNode &def_node, unique_ptr<DefAnalysis> def_analysis) {
  def_analyzes_.insert(make_pair(&def_node, move(def_analysis)));
  assert(!scopes_stack_.empty());
  Scope &current_scope = *(scopes_stack_.back());
  const string &id = def_node.GetNameToken().GetValue();
  const bool is_id_already_exists =
      !current_scope.GetIdDefs().insert(make_pair(id, &def_node)).second;

  if (is_id_already_exists) {
    unique_ptr<SemanticError> error(
        new DuplicateDefError(GetCurrentFilePath(), def_node));
    throw SemanticErrorException(move(error));
  }
}

bool SimpleSemanticAnalyzer::Impl::IsCurrentScopeGlobal() {
  return scopes_stack_.size() == 1;
}

bool SimpleSemanticAnalyzer::Impl::IsWithinImportProgram() {
  return file_scopes_stack_.size() > 1;
}

bool SimpleSemanticAnalyzer::Impl::IsTypeConvertible(const DataType &dest,
                                                     const DataType &src) {
  return data_type_converter_.IsConvertable(dest, src);
}

const path &SimpleSemanticAnalyzer::Impl::GetCurrentFilePath() {
  assert(!file_scopes_stack_.empty());
  return file_scopes_stack_.back()->GetFilePath();
}
}
}
