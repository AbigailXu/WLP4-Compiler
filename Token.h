#ifndef CS241_SCANNER_H
#define CS241_SCANNER_H
#include <bitset>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

enum Type {
  INT,
  PTR,
  NOT_DCL,
  NOT_FACTOR,
  NOT_EXPR,
  NOT_TERM,
  ID,
  NUM,
  NUL,
  undefined_factor,
  undefined_expr,
  err
};

enum FactorRule {
  notFactor,
  factor__ID,
  factor__NUM,
  factor__NULL,
  factor__AMP_lvalue,
  factor__STAR_factor,
  factor__LPAREN_expr_RPAREN,
  factor__ID_LPAREN_RPAREN,
  factor__NEW_INT_LBRACK_expr_RBRACK,
  factor__ID_LPAREN_arglist_RPAREN
};

enum ExprRule {
  notExpr,
  expr__term,
  expr__expr_PLUS_term,
  expr__expr_MINUS_term
};

enum TermRule {
  notTerm,
  term__factor,
  term__term_STAR_factor,
  term__term_SLASH_factor,
  term__term_PCT_factor
};

enum LvalueRule {
  notLvalue,
  lvalue__ID,
  lvalue__STAR_factor,
  lvalue__LPAREN_lvalue_RPAREN // factor ?
};

enum DclsRule {
  notDcls,
  dcls__,
  dcls__dcls_dcl_BECOMES_NUM_SEMI,
  dcls__dcls_dcl_BECOMES_NULL_SEMI
};

enum StatementRule {
  notStatement,
  statement__lvalue_BECOMES_expr_SEMI,
  statement__IF_LPAREN_test_RPAREN_LBRACE_statements_RBRACE_ELSE_LBRACE_statements_RBRACE,
  statement__WHILE_LPAREN_test_RPAREN_LBRACE_statements_RBRACE,
  statement__PRINTLN_LPAREN_expr_RPAREN_SEMI,
  statement__DELETE_LBRACK_RBRACK_expr_SEMI
};

enum TestRule {
  notTest,
  test__expr_EQ_expr,
  test__expr_NE_expr,
  test__expr_LT_expr,
  test__expr_LE_expr,
  test__expr_GE_expr,
  test__expr_GT_expr
};

std::string getTypeString(Type t);

class Token {
public:
  std::string Symbol = "undefined";
  std::string Contex = "undefined";
  std::vector<Token> Children;
  Token();
  Token(std::string symbol);
  Token(std::string symbol, std::string contex);
  ~Token(){};

  int getChildCount();
  int getArgLength();

  bool isTerminal();
  bool is(std::string compareTo);

  Type getDclType();

  FactorRule getFactorRule();
  ExprRule getExprRule();
  TermRule getTermRule();
  LvalueRule getLvalueRule();
  DclsRule getDclsRule();
  StatementRule getStatementRule();
  TestRule getTestRule();

  std::string getDclId();
  std::string getProcedureName();

  void print();
};

#endif