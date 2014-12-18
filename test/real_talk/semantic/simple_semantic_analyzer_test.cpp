
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <iterator>
#include "real_talk/lexer/token_info.h"
#include "real_talk/parser/parser_factory.h"
#include "real_talk/parser/program_node.h"
#include "real_talk/parser/var_def_without_init_node.h"
#include "real_talk/parser/var_def_with_init_node.h"
#include "real_talk/parser/id_node.h"
#include "real_talk/parser/string_node.h"
#include "real_talk/parser/int_node.h"
#include "real_talk/parser/long_node.h"
#include "real_talk/parser/double_node.h"
#include "real_talk/parser/char_node.h"
#include "real_talk/parser/bool_node.h"
#include "real_talk/parser/assign_node.h"
#include "real_talk/parser/expr_stmt_node.h"
#include "real_talk/parser/int_data_type_node.h"
#include "real_talk/parser/long_data_type_node.h"
#include "real_talk/parser/string_data_type_node.h"
#include "real_talk/parser/double_data_type_node.h"
#include "real_talk/parser/char_data_type_node.h"
#include "real_talk/parser/bool_data_type_node.h"
#include "real_talk/parser/void_data_type_node.h"
#include "real_talk/parser/array_data_type_node.h"
#include "real_talk/parser/bounded_array_data_type_node.h"
#include "real_talk/parser/func_def_node.h"
#include "real_talk/parser/arg_def_node.h"
#include "real_talk/parser/return_value_node.h"
#include "real_talk/parser/scope_node.h"
#include "real_talk/parser/if_else_if_node.h"
#include "real_talk/parser/if_else_if_else_node.h"
#include "real_talk/parser/import_node.h"
#include "real_talk/semantic/int_data_type.h"
#include "real_talk/semantic/long_data_type.h"
#include "real_talk/semantic/double_data_type.h"
#include "real_talk/semantic/bool_data_type.h"
#include "real_talk/semantic/string_data_type.h"
#include "real_talk/semantic/char_data_type.h"
#include "real_talk/semantic/void_data_type.h"
#include "real_talk/semantic/array_data_type.h"
#include "real_talk/semantic/bounded_array_data_type.h"
#include "real_talk/semantic/int_lit.h"
#include "real_talk/semantic/long_lit.h"
#include "real_talk/semantic/double_lit.h"
#include "real_talk/semantic/bool_lit.h"
#include "real_talk/semantic/string_lit.h"
#include "real_talk/semantic/var_def_analysis.h"
#include "real_talk/semantic/func_def_analysis.h"
#include "real_talk/semantic/arg_def_analysis.h"
#include "real_talk/semantic/semantic_analysis.h"
#include "real_talk/semantic/simple_semantic_analyzer.h"
#include "real_talk/semantic/global_data_storage.h"
#include "real_talk/semantic/local_data_storage.h"

using std::vector;
using std::string;
using std::unordered_map;
using std::unique_ptr;
using std::shared_ptr;
using std::move;
using std::make_pair;
using std::make_move_iterator;
using std::pair;
using testing::Test;
using testing::Return;
using testing::NiceMock;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::InSequence;
using testing::_;
using real_talk::lexer::TokenInfo;
using real_talk::lexer::Token;
using real_talk::parser::ProgramNode;
using real_talk::parser::StmtNode;
using real_talk::parser::ExprNode;
using real_talk::parser::DefNode;
using real_talk::parser::ArgDefNode;
using real_talk::parser::VarDefWithoutInitNode;
using real_talk::parser::VarDefWithInitNode;
using real_talk::parser::IdNode;
using real_talk::parser::ReturnValueNode;
using real_talk::parser::DataTypeNode;
using real_talk::parser::IntDataTypeNode;
using real_talk::parser::LongDataTypeNode;
using real_talk::parser::StringDataTypeNode;
using real_talk::parser::DoubleDataTypeNode;
using real_talk::parser::CharDataTypeNode;
using real_talk::parser::BoolDataTypeNode;
using real_talk::parser::VoidDataTypeNode;
using real_talk::parser::ArrayDataTypeNode;
using real_talk::parser::BoundedArrayDataTypeNode;
using real_talk::parser::StringNode;
using real_talk::parser::BoolNode;
using real_talk::parser::IntNode;
using real_talk::parser::LongNode;
using real_talk::parser::DoubleNode;
using real_talk::parser::AssignNode;
using real_talk::parser::ExprStmtNode;
using real_talk::parser::ScopeNode;
using real_talk::parser::FuncDefNode;
using real_talk::parser::IfNode;
using real_talk::parser::ElseIfNode;
using real_talk::parser::IfElseIfNode;
using real_talk::parser::IfElseIfElseNode;
using real_talk::parser::ImportNode;
using real_talk::parser::ParserFactory;
using real_talk::parser::Parser;

namespace real_talk {
namespace semantic {

struct TestImportProgram {
  string file_path;
  shared_ptr<ProgramNode> node;
};

struct TestProgram {
  unique_ptr<ProgramNode> node;
  SemanticAnalysis analysis;
  vector<TestImportProgram> test_import_programs;
};

class ParserMock: public Parser {
 public:
  MOCK_METHOD0(Parse, shared_ptr<ProgramNode> ());
};

class ParserFactoryMock: public ParserFactory {
 public:
  virtual unique_ptr<Parser> Create(const string &file_path) const override {
    return unique_ptr<Parser>(Create_(file_path));
  }

  MOCK_CONST_METHOD1(Create_, Parser* (const string&));
};

class SimpleSemanticAnalyzerTest: public Test {
 protected:
  virtual void SetUp() override {
  }

  virtual void TearDown() override {
  }

  void TestAnalyze(const TestProgram &test_program) {
    ParserFactoryMock *parser_factory_mock = new NiceMock<ParserFactoryMock>();
    unique_ptr<ParserFactory> parser_factory(parser_factory_mock);

    {
      InSequence sequence;

      for (const TestImportProgram &test_import_program:
               test_program.test_import_programs) {
        ParserMock *parser_mock = new NiceMock<ParserMock>();
        unique_ptr<Parser> parser(parser_mock);
        EXPECT_CALL(*parser_factory_mock,
                    Create_(test_import_program.file_path))
            .Times(1)
            .WillOnce(Return(parser_mock))
            .RetiresOnSaturation();
        EXPECT_CALL(*parser_mock, Parse())
            .Times(1)
            .WillOnce(Return(test_import_program.node))
            .RetiresOnSaturation();
        parser.release();
      }
    }

    SimpleSemanticAnalyzer analyzer(*(test_program.node), *parser_factory);
    SemanticAnalysis actual_analysis = analyzer.Analyze();
    ASSERT_EQ(test_program.analysis, actual_analysis);
  }
};

TEST_F(SimpleSemanticAnalyzerTest, VarDefWithoutInit) {
  struct TestData {
    unique_ptr<DataTypeNode> data_type_node;
    unique_ptr<DataType> data_type;
    SemanticAnalysis::ExprAnalyzes expr_analyzes;
    SemanticAnalysis::LitAnalyzes lit_analyzes;
  };

  vector<TestData> test_data_suits;

  {
    unique_ptr<DataTypeNode> data_type_node(new IntDataTypeNode(
        TokenInfo(Token::kIntType, "int", UINT32_C(0), UINT32_C(0))));
    unique_ptr<DataType> data_type(new IntDataType());
    test_data_suits.push_back({move(data_type_node), move(data_type), {}, {}});
  }

  {
    unique_ptr<DataTypeNode> data_type_node(new LongDataTypeNode(
        TokenInfo(Token::kLongType, "long", UINT32_C(0), UINT32_C(0))));
    unique_ptr<DataType> data_type(new LongDataType());
    test_data_suits.push_back({move(data_type_node), move(data_type), {}, {}});
  }

  {
    unique_ptr<DataTypeNode> data_type_node(new DoubleDataTypeNode(
        TokenInfo(Token::kDoubleType, "double", UINT32_C(0), UINT32_C(0))));
    unique_ptr<DataType> data_type(new DoubleDataType());
    test_data_suits.push_back({move(data_type_node), move(data_type), {}, {}});
  }

  {
    unique_ptr<DataTypeNode> data_type_node(new BoolDataTypeNode(
        TokenInfo(Token::kBoolType, "bool", UINT32_C(0), UINT32_C(0))));
    unique_ptr<DataType> data_type(new BoolDataType());
    test_data_suits.push_back({move(data_type_node), move(data_type), {}, {}});
  }

  {
    unique_ptr<DataTypeNode> data_type_node(new CharDataTypeNode(
        TokenInfo(Token::kCharType, "char", UINT32_C(0), UINT32_C(0))));
    unique_ptr<DataType> data_type(new CharDataType());
    test_data_suits.push_back({move(data_type_node), move(data_type), {}, {}});
  }

  {
    unique_ptr<DataTypeNode> data_type_node(new StringDataTypeNode(
        TokenInfo(Token::kStringType, "string", UINT32_C(0), UINT32_C(0))));
    unique_ptr<DataType> data_type(new StringDataType());
    test_data_suits.push_back({move(data_type_node), move(data_type), {}, {}});
  }

  {
    unique_ptr<DataTypeNode> data_type_node(new VoidDataTypeNode(
        TokenInfo(Token::kVoidType, "void", UINT32_C(0), UINT32_C(0))));
    unique_ptr<DataType> data_type(new VoidDataType());
    test_data_suits.push_back({move(data_type_node), move(data_type), {}, {}});
  }

  {
    unique_ptr<DataTypeNode> element_data_type_node(new IntDataTypeNode(
        TokenInfo(Token::kIntType, "int", UINT32_C(0), UINT32_C(0))));
    unique_ptr<DataTypeNode> data_type_node(new ArrayDataTypeNode(
        move(element_data_type_node),
        TokenInfo(Token::kSubscriptStart, "[", UINT32_C(1), UINT32_C(1)),
        TokenInfo(Token::kSubscriptEnd, "]", UINT32_C(2), UINT32_C(2))));
    unique_ptr<DataType> data_type(new ArrayDataType(
        unique_ptr<DataType>(new IntDataType())));
    test_data_suits.push_back({move(data_type_node), move(data_type), {}, {}});
  }

  {
    unique_ptr<DataTypeNode> element_data_type_node(new LongDataTypeNode(
        TokenInfo(Token::kLongType, "long", UINT32_C(0), UINT32_C(0))));
    IntNode *size_node_ptr =
        new IntNode(TokenInfo(Token::kIntLit, "7", UINT32_C(2), UINT32_C(2)));
    unique_ptr<IntNode> size_node(size_node_ptr);
    unique_ptr<DataTypeNode> array_data_type_node(
        new BoundedArrayDataTypeNode(
            move(element_data_type_node),
            TokenInfo(Token::kSubscriptStart, "[", UINT32_C(1), UINT32_C(1)),
            move(size_node),
            TokenInfo(Token::kSubscriptEnd, "]", UINT32_C(3), UINT32_C(3))));
    unique_ptr<DataType> element_data_type(new LongDataType());
    unique_ptr<IntLit> size(new IntLit(INT32_C(7)));
    unique_ptr<DataType> array_data_type(new BoundedArrayDataType(
        move(element_data_type), size->Clone()));
    SemanticAnalysis::ExprAnalyzes expr_analyzes;
    unique_ptr<DataType> size_data_type(new IntDataType());
    ExprAnalysis expr_analysis(move(size_data_type));
    expr_analyzes.insert(make_pair(size_node_ptr, move(expr_analysis)));
    SemanticAnalysis::LitAnalyzes lit_analyzes;
    LitAnalysis lit_analysis(size->Clone());
    lit_analyzes.insert(make_pair(size_node_ptr, move(lit_analysis)));
    TestData test_data = {move(array_data_type_node),
                          move(array_data_type),
                          move(expr_analyzes),
                          move(lit_analyzes)};
    test_data_suits.push_back(move(test_data));
  }

  for (TestData &test_data: test_data_suits) {
    vector< unique_ptr<StmtNode> > stmt_nodes;
    VarDefWithoutInitNode *def_node_ptr = new VarDefWithoutInitNode(
        move(test_data.data_type_node),
        TokenInfo(Token::kName, "var", UINT32_C(4), UINT32_C(4)),
        TokenInfo(Token::kStmtEnd, ";", UINT32_C(5), UINT32_C(5)));
    unique_ptr<StmtNode> def_node(def_node_ptr);
    stmt_nodes.push_back(move(def_node));
    unique_ptr<ProgramNode> program_node(new ProgramNode(move(stmt_nodes)));

    SemanticAnalysis::DefAnalyzes def_analyzes;
    unique_ptr<DataStorage> data_storage(new GlobalDataStorage());
    unique_ptr<DefAnalysis> def_analysis(
        new VarDefAnalysis(move(test_data.data_type), move(data_storage)));
    def_analyzes.insert(make_pair(def_node_ptr, move(def_analysis)));

    vector< unique_ptr<SemanticError> > errors;
    SemanticAnalysis::IdAnalyzes id_analyzes;
    SemanticAnalysis::ImportAnalyzes import_analyzes;
    SemanticAnalysis analysis(move(errors),
                              move(def_analyzes),
                              move(test_data.expr_analyzes),
                              move(test_data.lit_analyzes),
                              import_analyzes,
                              id_analyzes);
    vector<TestImportProgram> test_import_programs;

    TestAnalyze({move(program_node), move(analysis), test_import_programs});
  }
}

TEST_F(SimpleSemanticAnalyzerTest, VarDefWithInit) {
  vector< unique_ptr<StmtNode> > stmt_nodes;
  unique_ptr<DataTypeNode> var_data_type_node(new IntDataTypeNode(
      TokenInfo(Token::kIntType, "int", UINT32_C(0), UINT32_C(0))));
  IntNode *value_node = new IntNode(
      TokenInfo(Token::kIntLit, "7", UINT32_C(3), UINT32_C(3)));
  VarDefWithInitNode *def_node_ptr = new VarDefWithInitNode(
      move(var_data_type_node),
      TokenInfo(Token::kName, "var", UINT32_C(1), UINT32_C(1)),
      TokenInfo(Token::kAssignOp, "=", UINT32_C(2), UINT32_C(2)),
      unique_ptr<ExprNode>(value_node),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(4), UINT32_C(4)));
  unique_ptr<StmtNode> def_node(def_node_ptr);
  stmt_nodes.push_back(move(def_node));
  unique_ptr<ProgramNode> program_node(new ProgramNode(move(stmt_nodes)));

  SemanticAnalysis::DefAnalyzes def_analyzes;
  unique_ptr<DataType> var_data_type(new IntDataType());
  unique_ptr<DataStorage> var_data_storage(new GlobalDataStorage());
  unique_ptr<DefAnalysis> def_analysis(new VarDefAnalysis(
      move(var_data_type), move(var_data_storage)));
  def_analyzes.insert(make_pair(def_node_ptr, move(def_analysis)));

  SemanticAnalysis::ExprAnalyzes expr_analyzes;
  unique_ptr<DataType> value_data_type(new IntDataType());
  ExprAnalysis expr_analysis(move(value_data_type));
  expr_analyzes.insert(make_pair(value_node, move(expr_analysis)));

  SemanticAnalysis::LitAnalyzes lit_analyzes;
  LitAnalysis lit_analysis(unique_ptr<Lit>(new IntLit(INT32_C(7))));
  lit_analyzes.insert(make_pair(value_node, move(lit_analysis)));

  SemanticAnalysis::IdAnalyzes id_analyzes;
  SemanticAnalysis::ImportAnalyzes import_analyzes;
  vector< unique_ptr<SemanticError> > errors;
  SemanticAnalysis analysis(move(errors),
                            move(def_analyzes),
                            move(expr_analyzes),
                            move(lit_analyzes),
                            import_analyzes,
                            id_analyzes);
  vector<TestImportProgram> test_import_programs;

  TestAnalyze({move(program_node), move(analysis), test_import_programs});
}

TEST_F(SimpleSemanticAnalyzerTest, Id) {
  vector< unique_ptr<StmtNode> > stmt_nodes;
  unique_ptr<DataTypeNode> var_data_type_node(new IntDataTypeNode(
      TokenInfo(Token::kIntType, "int", UINT32_C(0), UINT32_C(0))));
  VarDefWithoutInitNode *def_node_ptr = new VarDefWithoutInitNode(
      move(var_data_type_node),
      TokenInfo(Token::kName, "var", UINT32_C(1), UINT32_C(1)),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(2), UINT32_C(2)));
  unique_ptr<StmtNode> def_node(def_node_ptr);
  stmt_nodes.push_back(move(def_node));

  IdNode *id_node_ptr = new IdNode(
      TokenInfo(Token::kName, "var", UINT32_C(3), UINT32_C(3)));
  unique_ptr<ExprNode> id_node(id_node_ptr);
  unique_ptr<StmtNode> id_stmt_node(new ExprStmtNode(
      move(id_node),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(4), UINT32_C(4))));
  stmt_nodes.push_back(move(id_stmt_node));
  unique_ptr<ProgramNode> program_node(new ProgramNode(move(stmt_nodes)));

  SemanticAnalysis::DefAnalyzes def_analyzes;
  unique_ptr<DataType> var_data_type(new IntDataType());
  unique_ptr<DataStorage> var_data_storage(new GlobalDataStorage());
  unique_ptr<DefAnalysis> def_analysis(new VarDefAnalysis(
      move(var_data_type), move(var_data_storage)));
  def_analyzes.insert(make_pair(def_node_ptr, move(def_analysis)));

  SemanticAnalysis::ExprAnalyzes expr_analyzes;
  unique_ptr<DataType> expr_data_type(new IntDataType());
  ExprAnalysis expr_analysis(move(expr_data_type));
  expr_analyzes.insert(make_pair(id_node_ptr, move(expr_analysis)));

  SemanticAnalysis::IdAnalyzes id_analyzes;
  IdAnalysis id_analysis(def_node_ptr);
  id_analyzes.insert(make_pair(id_node_ptr, id_analysis));

  SemanticAnalysis::LitAnalyzes lit_analyzes;
  SemanticAnalysis::ImportAnalyzes import_analyzes;
  vector< unique_ptr<SemanticError> > errors;
  SemanticAnalysis analysis(move(errors),
                            move(def_analyzes),
                            move(expr_analyzes),
                            move(lit_analyzes),
                            import_analyzes,
                            id_analyzes);
  vector<TestImportProgram> test_import_programs;

  TestAnalyze({move(program_node), move(analysis), test_import_programs});
}

TEST_F(SimpleSemanticAnalyzerTest, FuncDefWithReturnValue) {
  vector< unique_ptr<StmtNode> > stmt_nodes;
  vector< unique_ptr<ArgDefNode> > arg_def_nodes;
  unique_ptr<DataTypeNode> arg_data_type_node(new IntDataTypeNode(
      TokenInfo(Token::kIntType, "int", UINT32_C(3), UINT32_C(3))));
  ArgDefNode *arg_def_node_ptr = new ArgDefNode(
      move(arg_data_type_node),
      TokenInfo(Token::kName, "name", UINT32_C(4), UINT32_C(4)));
  unique_ptr<ArgDefNode> arg_def_node(arg_def_node_ptr);
  arg_def_nodes.push_back(move(arg_def_node));
  vector<TokenInfo> arg_separator_tokens;

  vector< unique_ptr<StmtNode> > body_stmt_nodes;
  IdNode *return_expr_node_ptr =
      new IdNode(TokenInfo(Token::kName, "name", UINT32_C(8), UINT32_C(8)));
  unique_ptr<ExprNode> return_expr_node(return_expr_node_ptr);
  unique_ptr<StmtNode> return_stmt_node(new ReturnValueNode(
      TokenInfo(Token::kReturn, "return", UINT32_C(7), UINT32_C(7)),
      move(return_expr_node),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(9), UINT32_C(9))));
  body_stmt_nodes.push_back(move(return_stmt_node));

  unique_ptr<ScopeNode> body_node(new ScopeNode(
      TokenInfo(Token::kScopeStart, "{", UINT32_C(6), UINT32_C(6)),
      move(body_stmt_nodes),
      TokenInfo(Token::kScopeEnd, "}", UINT32_C(10), UINT32_C(10))));

  unique_ptr<DataTypeNode> func_data_type_node(new IntDataTypeNode(
      TokenInfo(Token::kIntType, "int", UINT32_C(0), UINT32_C(0))));
  FuncDefNode *func_def_node_ptr = new FuncDefNode(
      move(func_data_type_node),
      TokenInfo(Token::kName, "name", UINT32_C(1), UINT32_C(1)),
      TokenInfo(Token::kGroupStart, "(", UINT32_C(2), UINT32_C(2)),
      move(arg_def_nodes),
      arg_separator_tokens,
      TokenInfo(Token::kGroupEnd, ")", UINT32_C(5), UINT32_C(5)),
      move(body_node));
  unique_ptr<StmtNode> func_def_node(func_def_node_ptr);
  stmt_nodes.push_back(move(func_def_node));
  unique_ptr<ProgramNode> program_node(new ProgramNode(move(stmt_nodes)));

  SemanticAnalysis::DefAnalyzes def_analyzes;
  unique_ptr<DataType> func_data_type(new IntDataType());
  unique_ptr<DefAnalysis> func_def_analysis(
      new FuncDefAnalysis(move(func_data_type)));
  def_analyzes.insert(
      make_pair(func_def_node_ptr, move(func_def_analysis)));
  unique_ptr<DataType> arg_data_type(new IntDataType());
  unique_ptr<DefAnalysis> arg_def_analysis(
      new ArgDefAnalysis(move(arg_data_type)));
  def_analyzes.insert(make_pair(arg_def_node_ptr, move(arg_def_analysis)));

  SemanticAnalysis::ExprAnalyzes expr_analyzes;
  unique_ptr<DataType> return_expr_data_type(new IntDataType);
  ExprAnalysis return_expr_analysis(move(return_expr_data_type));
  expr_analyzes.insert(make_pair(return_expr_node_ptr,
                                 move(return_expr_analysis)));

  SemanticAnalysis::IdAnalyzes id_analyzes;
  IdAnalysis id_analysis(arg_def_node_ptr);
  id_analyzes.insert(make_pair(return_expr_node_ptr, id_analysis));

  SemanticAnalysis::LitAnalyzes lit_analyzes;
  SemanticAnalysis::ImportAnalyzes import_analyzes;
  vector< unique_ptr<SemanticError> > errors;
  SemanticAnalysis analysis(move(errors),
                            move(def_analyzes),
                            move(expr_analyzes),
                            move(lit_analyzes),
                            import_analyzes,
                            id_analyzes);
  vector<TestImportProgram> test_import_programs;

  TestAnalyze({move(program_node), move(analysis), test_import_programs});
}

TEST_F(SimpleSemanticAnalyzerTest, FuncDefWithoutReturnValue) {
  vector< unique_ptr<StmtNode> > stmt_nodes;
  vector< unique_ptr<ArgDefNode> > arg_def_nodes;
  unique_ptr<DataTypeNode> arg_data_type_node(new IntDataTypeNode(
      TokenInfo(Token::kIntType, "int", UINT32_C(3), UINT32_C(3))));
  ArgDefNode *arg_def_node_ptr = new ArgDefNode(
      move(arg_data_type_node),
      TokenInfo(Token::kName, "name", UINT32_C(4), UINT32_C(4)));
  unique_ptr<ArgDefNode> arg_def_node(arg_def_node_ptr);
  arg_def_nodes.push_back(move(arg_def_node));
  vector<TokenInfo> arg_separator_tokens;

  vector< unique_ptr<StmtNode> > body_stmt_nodes;
  unique_ptr<ScopeNode> body_node(new ScopeNode(
      TokenInfo(Token::kScopeStart, "{", UINT32_C(6), UINT32_C(6)),
      move(body_stmt_nodes),
      TokenInfo(Token::kScopeEnd, "}", UINT32_C(7), UINT32_C(7))));

  unique_ptr<DataTypeNode> func_data_type_node(new VoidDataTypeNode(
      TokenInfo(Token::kVoidType, "void", UINT32_C(0), UINT32_C(0))));
  FuncDefNode *func_def_node_ptr = new FuncDefNode(
      move(func_data_type_node),
      TokenInfo(Token::kName, "name", UINT32_C(1), UINT32_C(1)),
      TokenInfo(Token::kGroupStart, "(", UINT32_C(2), UINT32_C(2)),
      move(arg_def_nodes),
      arg_separator_tokens,
      TokenInfo(Token::kGroupEnd, ")", UINT32_C(5), UINT32_C(5)),
      move(body_node));
  unique_ptr<StmtNode> func_def_node(func_def_node_ptr);
  stmt_nodes.push_back(move(func_def_node));
  unique_ptr<ProgramNode> program_node(new ProgramNode(move(stmt_nodes)));

  SemanticAnalysis::DefAnalyzes def_analyzes;
  unique_ptr<DataType> func_data_type(new VoidDataType());
  unique_ptr<DefAnalysis> func_def_analysis(
      new FuncDefAnalysis(move(func_data_type)));
  def_analyzes.insert(
      make_pair(func_def_node_ptr, move(func_def_analysis)));
  unique_ptr<DataType> arg_data_type(new IntDataType());
  unique_ptr<DefAnalysis> arg_def_analysis(
      new ArgDefAnalysis(move(arg_data_type)));
  def_analyzes.insert(make_pair(arg_def_node_ptr, move(arg_def_analysis)));

  SemanticAnalysis::IdAnalyzes id_analyzes;
  SemanticAnalysis::ExprAnalyzes expr_analyzes;
  SemanticAnalysis::LitAnalyzes lit_analyzes;
  SemanticAnalysis::ImportAnalyzes import_analyzes;
  vector< unique_ptr<SemanticError> > errors;
  SemanticAnalysis analysis(move(errors),
                            move(def_analyzes),
                            move(expr_analyzes),
                            move(lit_analyzes),
                            import_analyzes,
                            id_analyzes);
  vector<TestImportProgram> test_import_programs;

  TestAnalyze({move(program_node), move(analysis), test_import_programs});
}

TEST_F(SimpleSemanticAnalyzerTest, IfElseIf) {
  vector< unique_ptr<StmtNode> > stmt_nodes;
  unique_ptr<DataTypeNode> var_data_type_node1(new BoolDataTypeNode(
      TokenInfo(Token::kBoolType, "bool", UINT32_C(0), UINT32_C(0))));
  VarDefWithoutInitNode *var_def_node_ptr1 = new VarDefWithoutInitNode(
      move(var_data_type_node1),
      TokenInfo(Token::kName, "name", UINT32_C(1), UINT32_C(1)),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(2), UINT32_C(2)));
  unique_ptr<StmtNode> var_def_node1(var_def_node_ptr1);
  stmt_nodes.push_back(move(var_def_node1));

  vector< unique_ptr<StmtNode> > body_stmt_nodes1;
  unique_ptr<DataTypeNode> var_data_type_node2(new BoolDataTypeNode(
      TokenInfo(Token::kBoolType, "bool", UINT32_C(8), UINT32_C(8))));
  VarDefWithoutInitNode *var_def_node_ptr2 = new VarDefWithoutInitNode(
      move(var_data_type_node2),
      TokenInfo(Token::kName, "name", UINT32_C(9), UINT32_C(9)),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(10), UINT32_C(10)));
  unique_ptr<StmtNode> var_def_node2(var_def_node_ptr2);
  body_stmt_nodes1.push_back(move(var_def_node2));
  unique_ptr<ScopeNode> if_body1(new ScopeNode(
      TokenInfo(Token::kScopeStart, "{", UINT32_C(7), UINT32_C(7)),
      move(body_stmt_nodes1),
      TokenInfo(Token::kScopeEnd, "}", UINT32_C(11), UINT32_C(11))));

  IdNode *if_cond_ptr1 = new IdNode(
      TokenInfo(Token::kName, "name", UINT32_C(5), UINT32_C(5)));
  unique_ptr<ExprNode> if_cond1(if_cond_ptr1);
  unique_ptr<IfNode> if_node1(new IfNode(
      TokenInfo(Token::kIf, "if", UINT32_C(3), UINT32_C(3)),
      TokenInfo(Token::kGroupStart, "(", UINT32_C(4), UINT32_C(4)),
      move(if_cond1),
      TokenInfo(Token::kGroupEnd, ")", UINT32_C(6), UINT32_C(6)),
      move(if_body1)));

  vector< unique_ptr<ElseIfNode> > else_if_nodes;
  vector< unique_ptr<StmtNode> > body_stmt_nodes2;
  unique_ptr<DataTypeNode> var_data_type_node3(new BoolDataTypeNode(
      TokenInfo(Token::kBoolType, "bool", UINT32_C(18), UINT32_C(18))));
  VarDefWithoutInitNode *var_def_node_ptr3 = new VarDefWithoutInitNode(
      move(var_data_type_node3),
      TokenInfo(Token::kName, "name", UINT32_C(19), UINT32_C(19)),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(20), UINT32_C(20)));
  unique_ptr<StmtNode> var_def_node3(var_def_node_ptr3);
  body_stmt_nodes2.push_back(move(var_def_node3));
  unique_ptr<ScopeNode> if_body2(new ScopeNode(
      TokenInfo(Token::kScopeStart, "{", UINT32_C(17), UINT32_C(17)),
      move(body_stmt_nodes2),
      TokenInfo(Token::kScopeEnd, "}", UINT32_C(21), UINT32_C(21))));

  IdNode *if_cond_ptr2 = new IdNode(
      TokenInfo(Token::kName, "name", UINT32_C(15), UINT32_C(15)));
  unique_ptr<ExprNode> if_cond2(if_cond_ptr2);
  unique_ptr<IfNode> if_node2(new IfNode(
      TokenInfo(Token::kIf, "if", UINT32_C(13), UINT32_C(13)),
      TokenInfo(Token::kGroupStart, "(", UINT32_C(14), UINT32_C(14)),
      move(if_cond2),
      TokenInfo(Token::kGroupEnd, ")", UINT32_C(16), UINT32_C(16)),
      move(if_body2)));
  unique_ptr<ElseIfNode> else_if_node(new ElseIfNode(
      TokenInfo(Token::kElse, "else", UINT32_C(12), UINT32_C(12)),
      move(if_node2)));
  else_if_nodes.push_back(move(else_if_node));

  unique_ptr<StmtNode> if_else_if_node(
      new IfElseIfNode(move(if_node1), move(else_if_nodes)));
  stmt_nodes.push_back(move(if_else_if_node));
  unique_ptr<ProgramNode> program_node(new ProgramNode(move(stmt_nodes)));

  SemanticAnalysis::DefAnalyzes def_analyzes;
  unique_ptr<DataType> var_data_type1(new BoolDataType());
  unique_ptr<DataStorage> var_storage1(new GlobalDataStorage());
  unique_ptr<DefAnalysis> var_def_analysis1(
      new VarDefAnalysis(move(var_data_type1), move(var_storage1)));
  def_analyzes.insert(make_pair(var_def_node_ptr1, move(var_def_analysis1)));
  unique_ptr<DataType> var_data_type2(new BoolDataType());
  unique_ptr<DataStorage> var_storage2(new LocalDataStorage());
  unique_ptr<DefAnalysis> var_def_analysis2(
      new VarDefAnalysis(move(var_data_type2), move(var_storage2)));
  def_analyzes.insert(make_pair(var_def_node_ptr2, move(var_def_analysis2)));
  unique_ptr<DataType> var_data_type3(new BoolDataType());
  unique_ptr<DataStorage> var_storage3(new LocalDataStorage());
  unique_ptr<DefAnalysis> var_def_analysis3(
      new VarDefAnalysis(move(var_data_type3), move(var_storage3)));
  def_analyzes.insert(make_pair(var_def_node_ptr3, move(var_def_analysis3)));

  SemanticAnalysis::ExprAnalyzes expr_analyzes;
  unique_ptr<DataType> if_cond_data_type1(new BoolDataType());
  ExprAnalysis if_cond_expr_analysis1(move(if_cond_data_type1));
  expr_analyzes.insert(make_pair(if_cond_ptr1, move(if_cond_expr_analysis1)));
  unique_ptr<DataType> if_cond_data_type2(new BoolDataType());
  ExprAnalysis if_cond_expr_analysis2(move(if_cond_data_type2));
  expr_analyzes.insert(make_pair(if_cond_ptr2, move(if_cond_expr_analysis2)));

  SemanticAnalysis::IdAnalyzes id_analyzes;
  IdAnalysis id_analysis1(var_def_node_ptr1);
  id_analyzes.insert(make_pair(if_cond_ptr1, id_analysis1));
  IdAnalysis id_analysis2(var_def_node_ptr1);
  id_analyzes.insert(make_pair(if_cond_ptr2, id_analysis2));

  SemanticAnalysis::LitAnalyzes lit_analyzes;
  SemanticAnalysis::ImportAnalyzes import_analyzes;
  vector< unique_ptr<SemanticError> > errors;
  SemanticAnalysis analysis(move(errors),
                            move(def_analyzes),
                            move(expr_analyzes),
                            move(lit_analyzes),
                            import_analyzes,
                            id_analyzes);
  vector<TestImportProgram> test_import_programs;

  TestAnalyze({move(program_node), move(analysis), test_import_programs});
}

TEST_F(SimpleSemanticAnalyzerTest, IfElseIfElse) {
  vector< unique_ptr<StmtNode> > stmt_nodes;
  unique_ptr<DataTypeNode> var_data_type_node1(new BoolDataTypeNode(
      TokenInfo(Token::kBoolType, "bool", UINT32_C(0), UINT32_C(0))));
  VarDefWithoutInitNode *var_def_node_ptr1 = new VarDefWithoutInitNode(
      move(var_data_type_node1),
      TokenInfo(Token::kName, "name", UINT32_C(1), UINT32_C(1)),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(2), UINT32_C(2)));
  unique_ptr<StmtNode> var_def_node1(var_def_node_ptr1);
  stmt_nodes.push_back(move(var_def_node1));

  vector< unique_ptr<StmtNode> > body_stmt_nodes1;
  unique_ptr<DataTypeNode> var_data_type_node2(new BoolDataTypeNode(
      TokenInfo(Token::kBoolType, "bool", UINT32_C(8), UINT32_C(8))));
  VarDefWithoutInitNode *var_def_node_ptr2 = new VarDefWithoutInitNode(
      move(var_data_type_node2),
      TokenInfo(Token::kName, "name", UINT32_C(9), UINT32_C(9)),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(10), UINT32_C(10)));
  unique_ptr<StmtNode> var_def_node2(var_def_node_ptr2);
  body_stmt_nodes1.push_back(move(var_def_node2));
  unique_ptr<ScopeNode> if_body1(new ScopeNode(
      TokenInfo(Token::kScopeStart, "{", UINT32_C(7), UINT32_C(7)),
      move(body_stmt_nodes1),
      TokenInfo(Token::kScopeEnd, "}", UINT32_C(11), UINT32_C(11))));

  IdNode *if_cond_ptr1 = new IdNode(
      TokenInfo(Token::kName, "name", UINT32_C(5), UINT32_C(5)));
  unique_ptr<ExprNode> if_cond1(if_cond_ptr1);
  unique_ptr<IfNode> if_node1(new IfNode(
      TokenInfo(Token::kIf, "if", UINT32_C(3), UINT32_C(3)),
      TokenInfo(Token::kGroupStart, "(", UINT32_C(4), UINT32_C(4)),
      move(if_cond1),
      TokenInfo(Token::kGroupEnd, ")", UINT32_C(6), UINT32_C(6)),
      move(if_body1)));

  vector< unique_ptr<ElseIfNode> > else_if_nodes;
  vector< unique_ptr<StmtNode> > body_stmt_nodes2;
  unique_ptr<DataTypeNode> var_data_type_node3(new BoolDataTypeNode(
      TokenInfo(Token::kBoolType, "bool", UINT32_C(18), UINT32_C(18))));
  VarDefWithoutInitNode *var_def_node_ptr3 = new VarDefWithoutInitNode(
      move(var_data_type_node3),
      TokenInfo(Token::kName, "name", UINT32_C(19), UINT32_C(19)),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(20), UINT32_C(20)));
  unique_ptr<StmtNode> var_def_node3(var_def_node_ptr3);
  body_stmt_nodes2.push_back(move(var_def_node3));
  unique_ptr<ScopeNode> if_body2(new ScopeNode(
      TokenInfo(Token::kScopeStart, "{", UINT32_C(17), UINT32_C(17)),
      move(body_stmt_nodes2),
      TokenInfo(Token::kScopeEnd, "}", UINT32_C(21), UINT32_C(21))));

  IdNode *if_cond_ptr2 = new IdNode(
      TokenInfo(Token::kName, "name", UINT32_C(15), UINT32_C(15)));
  unique_ptr<ExprNode> if_cond2(if_cond_ptr2);
  unique_ptr<IfNode> if_node2(new IfNode(
      TokenInfo(Token::kIf, "if", UINT32_C(13), UINT32_C(13)),
      TokenInfo(Token::kGroupStart, "(", UINT32_C(14), UINT32_C(14)),
      move(if_cond2),
      TokenInfo(Token::kGroupEnd, ")", UINT32_C(16), UINT32_C(16)),
      move(if_body2)));
  unique_ptr<ElseIfNode> else_if_node(new ElseIfNode(
      TokenInfo(Token::kElse, "else", UINT32_C(12), UINT32_C(12)),
      move(if_node2)));
  else_if_nodes.push_back(move(else_if_node));

  vector< unique_ptr<StmtNode> > body_stmt_nodes3;
  unique_ptr<DataTypeNode> var_data_type_node4(new BoolDataTypeNode(
      TokenInfo(Token::kBoolType, "bool", UINT32_C(24), UINT32_C(24))));
  VarDefWithoutInitNode *var_def_node_ptr4 = new VarDefWithoutInitNode(
      move(var_data_type_node4),
      TokenInfo(Token::kName, "name", UINT32_C(25), UINT32_C(25)),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(26), UINT32_C(26)));
  unique_ptr<StmtNode> var_def_node4(var_def_node_ptr4);
  body_stmt_nodes3.push_back(move(var_def_node4));
  unique_ptr<ScopeNode> else_body(new ScopeNode(
      TokenInfo(Token::kScopeStart, "{", UINT32_C(23), UINT32_C(23)),
      move(body_stmt_nodes3),
      TokenInfo(Token::kScopeEnd, "}", UINT32_C(27), UINT32_C(27))));

  unique_ptr<StmtNode> if_else_if_node(new IfElseIfElseNode(
      move(if_node1),
      move(else_if_nodes),
      TokenInfo(Token::kElse, "else", UINT32_C(22), UINT32_C(22)),
      move(else_body)));
  stmt_nodes.push_back(move(if_else_if_node));
  unique_ptr<ProgramNode> program_node(new ProgramNode(move(stmt_nodes)));

  SemanticAnalysis::DefAnalyzes def_analyzes;
  unique_ptr<DataType> var_data_type1(new BoolDataType());
  unique_ptr<DataStorage> var_storage1(new GlobalDataStorage());
  unique_ptr<DefAnalysis> var_def_analysis1(
      new VarDefAnalysis(move(var_data_type1), move(var_storage1)));
  def_analyzes.insert(make_pair(var_def_node_ptr1, move(var_def_analysis1)));
  unique_ptr<DataType> var_data_type2(new BoolDataType());
  unique_ptr<DataStorage> var_storage2(new LocalDataStorage());
  unique_ptr<DefAnalysis> var_def_analysis2(
      new VarDefAnalysis(move(var_data_type2), move(var_storage2)));
  def_analyzes.insert(make_pair(var_def_node_ptr2, move(var_def_analysis2)));
  unique_ptr<DataType> var_data_type3(new BoolDataType());
  unique_ptr<DataStorage> var_storage3(new LocalDataStorage());
  unique_ptr<DefAnalysis> var_def_analysis3(
      new VarDefAnalysis(move(var_data_type3), move(var_storage3)));
  def_analyzes.insert(make_pair(var_def_node_ptr3, move(var_def_analysis3)));
  unique_ptr<DataType> var_data_type4(new BoolDataType());
  unique_ptr<DataStorage> var_storage4(new LocalDataStorage());
  unique_ptr<DefAnalysis> var_def_analysis4(
      new VarDefAnalysis(move(var_data_type4), move(var_storage4)));
  def_analyzes.insert(make_pair(var_def_node_ptr4, move(var_def_analysis4)));

  SemanticAnalysis::ExprAnalyzes expr_analyzes;
  unique_ptr<DataType> if_cond_data_type1(new BoolDataType());
  ExprAnalysis if_cond_expr_analysis1(move(if_cond_data_type1));
  expr_analyzes.insert(make_pair(if_cond_ptr1, move(if_cond_expr_analysis1)));
  unique_ptr<DataType> if_cond_data_type2(new BoolDataType());
  ExprAnalysis if_cond_expr_analysis2(move(if_cond_data_type2));
  expr_analyzes.insert(make_pair(if_cond_ptr2, move(if_cond_expr_analysis2)));

  SemanticAnalysis::IdAnalyzes id_analyzes;
  IdAnalysis id_analysis1(var_def_node_ptr1);
  id_analyzes.insert(make_pair(if_cond_ptr1, id_analysis1));
  IdAnalysis id_analysis2(var_def_node_ptr1);
  id_analyzes.insert(make_pair(if_cond_ptr2, id_analysis2));

  SemanticAnalysis::LitAnalyzes lit_analyzes;
  SemanticAnalysis::ImportAnalyzes import_analyzes;
  vector< unique_ptr<SemanticError> > errors;
  SemanticAnalysis analysis(move(errors),
                            move(def_analyzes),
                            move(expr_analyzes),
                            move(lit_analyzes),
                            import_analyzes,
                            id_analyzes);
  vector<TestImportProgram> test_import_programs;

  TestAnalyze({move(program_node), move(analysis), test_import_programs});
}

TEST_F(SimpleSemanticAnalyzerTest, Import) {
  vector< unique_ptr<StmtNode> > main_program_stmt_nodes;
  StringNode *file_path_node_ptr = new StringNode(
      TokenInfo(Token::kStringLit, "\"file.rt\"", UINT32_C(1), UINT32_C(1)));
  unique_ptr<StringNode> file_path_node(file_path_node_ptr);
  ImportNode *import_node_ptr = new ImportNode(
      TokenInfo(Token::kImport, "import", UINT32_C(0), UINT32_C(0)),
      move(file_path_node),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(2), UINT32_C(2)));
  unique_ptr<StmtNode> import_node(import_node_ptr);
  main_program_stmt_nodes.push_back(move(import_node));
  unique_ptr<ProgramNode> main_program_node(
      new ProgramNode(move(main_program_stmt_nodes)));

  vector<TestImportProgram> test_import_programs;
  vector< unique_ptr<StmtNode> > import_program_stmt_nodes;
  unique_ptr<DataTypeNode> var_data_type_node(new BoolDataTypeNode(
      TokenInfo(Token::kBoolType, "bool", UINT32_C(0), UINT32_C(0))));
  VarDefWithoutInitNode *var_def_node_ptr = new VarDefWithoutInitNode(
      move(var_data_type_node),
      TokenInfo(Token::kName, "name", UINT32_C(1), UINT32_C(1)),
      TokenInfo(Token::kStmtEnd, ";", UINT32_C(2), UINT32_C(2)));
  unique_ptr<StmtNode> var_def_node(var_def_node_ptr);
  import_program_stmt_nodes.push_back(move(var_def_node));

  shared_ptr<ProgramNode> import_program_node(
      new ProgramNode(move(import_program_stmt_nodes)));
  string import_program_file_path = "file.rt";
  TestImportProgram test_import_program =
      {import_program_file_path, import_program_node};
  test_import_programs.push_back(test_import_program);

  SemanticAnalysis::DefAnalyzes def_analyzes;
  unique_ptr<DataType> var_data_type(new BoolDataType());
  unique_ptr<DataStorage> var_storage(new GlobalDataStorage());
  unique_ptr<DefAnalysis> var_def_analysis(
      new VarDefAnalysis(move(var_data_type), move(var_storage)));
  def_analyzes.insert(make_pair(var_def_node_ptr, move(var_def_analysis)));

  SemanticAnalysis::LitAnalyzes lit_analyzes;
  unique_ptr<Lit> file_path_lit(new StringLit(import_program_file_path));
  LitAnalysis file_path_lit_analysis(move(file_path_lit));
  lit_analyzes.insert(
      make_pair(file_path_node_ptr, move(file_path_lit_analysis)));

  SemanticAnalysis::ImportAnalyzes import_analyzes;
  ImportAnalysis import_analysis(import_program_node);
  import_analyzes.insert(make_pair(import_node_ptr, import_analysis));

  SemanticAnalysis::ExprAnalyzes expr_analyzes;
  unique_ptr<DataType> file_path_expr_data_type(new StringDataType());
  ExprAnalysis file_path_expr_analysis(move(file_path_expr_data_type));
  expr_analyzes.insert(
      make_pair(file_path_node_ptr, move(file_path_expr_analysis)));

  SemanticAnalysis::IdAnalyzes id_analyzes;
  vector< unique_ptr<SemanticError> > errors;
  SemanticAnalysis analysis(move(errors),
                            move(def_analyzes),
                            move(expr_analyzes),
                            move(lit_analyzes),
                            move(import_analyzes),
                            id_analyzes);

  TestAnalyze({move(main_program_node), move(analysis), test_import_programs});
}
}
}
