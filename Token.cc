#include "Token.h"
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
using namespace std;

Token::Token() {}

Token::Token(string symbol) : Symbol(symbol) {}

Token::Token(string symbol, string contex) : Symbol(symbol), Contex(contex) {}

// get num of children
int Token::getChildCount() { return Children.size(); }

int Token::getArgLength() {
  if (Children.size() == 1) {
    return 1;
  } else {
    return 1 + Children[2].getArgLength();
  }
}

// is terminal ?
bool Token::isTerminal() {
  if (Symbol.compare("BOF") == 0 || Symbol.compare("BECOMES") == 0 ||
      Symbol.compare("COMMA") == 0 || Symbol.compare("ELSE") == 0 ||
      Symbol.compare("EOF") == 0 || Symbol.compare("EQ") == 0 ||
      Symbol.compare("GE") == 0 || Symbol.compare("GT") == 0 ||
      Symbol.compare("ID") == 0 || Symbol.compare("IF") == 0 ||
      Symbol.compare("INT") == 0 || Symbol.compare("LBRACE") == 0 ||
      Symbol.compare("LE") == 0 || Symbol.compare("LPAREN") == 0 ||
      Symbol.compare("LT") == 0 || Symbol.compare("MINUS") == 0 ||
      Symbol.compare("NE") == 0 || Symbol.compare("NUM") == 0 ||
      Symbol.compare("PCT") == 0 || Symbol.compare("PLUS") == 0 ||
      Symbol.compare("PRINTLN") == 0 || Symbol.compare("RBRACE") == 0 ||
      Symbol.compare("RETURN") == 0 || Symbol.compare("RPAREN") == 0 ||
      Symbol.compare("SEMI") == 0 || Symbol.compare("SLASH") == 0 ||
      Symbol.compare("STAR") == 0 || Symbol.compare("WAIN") == 0 ||
      Symbol.compare("WHILE") == 0 || Symbol.compare("AMP") == 0 ||
      Symbol.compare("LBRACK") == 0 || Symbol.compare("RBRACK") == 0 ||
      Symbol.compare("NEW") == 0 || Symbol.compare("DELETE") == 0 ||
      Symbol.compare("NULL") == 0) {
    return true;
  }
  return false;
}

bool Token::is(string compareTo) {
  if (Symbol.compare(compareTo) == 0) {
    return true;
  }
  return false;
}

// get dcl Type
Type Token::getDclType() {
  if (Symbol.compare("dcl") != 0) {
    return NOT_DCL;
  }
  if (Children[0].getChildCount() == 2) {
    return PTR;
  } else {
    return INT;
  }
}

// get dcl Id
string Token::getDclId() {
  if (Symbol.compare("dcl") != 0) {
    return "not_dcl";
  }
  return Children[1].Contex;
}

string Token::getProcedureName() {
  if (Symbol.compare("procedure") != 0 && Symbol.compare("main")) {
    return "not_procedure";
  }
  return Children[1].Contex;
}

// print Token
void Token::print() { cerr << Symbol << " " << Contex << endl; }

// Type -> string
string getTypeString(Type t) {
  switch (t) {
  case INT:
    return "int";
  case PTR:
    return "int*";
  case NOT_DCL:
    return "NOT_DCL";
  case NOT_FACTOR:
    return "NOT_FACTOR";
  case NOT_EXPR:
    return "NOT_EXPR";
  case NOT_TERM:
    return "NOT_TERM";
  case ID:
    return "ID";
  case NUM:
    return "NUM";
  case NUL:
    return "NUL";
  case undefined_factor:
    return "undefined_factor";
  case undefined_expr:
    return "undefined_exp";
  }
}

FactorRule Token::getFactorRule() {
  if (Children.size() == 1) {
    // factor → ID
    if (Children[0].is("ID")) {
      return factor__ID;
    }
    // factor → NUM
    else if (Children[0].is("NUM")) {
      return factor__NUM;
    }
    // factor → NULL
    else if (Children[0].is("NULL")) {
      return factor__NULL;
    }
  } else if (Children.size() == 2) {
    // factor → AMP lvalue
    if (Children[0].is("AMP")) {
      return factor__AMP_lvalue;
    }
    // factor → STAR factor
    else if (Children[0].is("STAR")) {
      return factor__STAR_factor;
    }
  } else if (Children.size() == 3) {
    // factor → LPAREN expr RPAREN
    if (Children[0].is("LPAREN")) {
      return factor__LPAREN_expr_RPAREN;
    }
    // factor → ID LPAREN RPAREN
    else if (Children[0].is("ID")) {
      return factor__ID_LPAREN_RPAREN;
    }
  } else if (Children.size() == 4) {
    // factor → ID LPAREN arglist RPAREN
    return factor__ID_LPAREN_arglist_RPAREN;
  } else if (Children[0].is("NEW")) {
    // factor → NEW INT LBRACK expr RBRACK
    return factor__NEW_INT_LBRACK_expr_RBRACK;
  }
  return notFactor;
}

ExprRule Token::getExprRule() {
  if (Children.size() == 1) {
    // expr → term
    return expr__term;
  }
  // expr → expr PLUS term
  else if (Children[1].is("PLUS")) {
    return expr__expr_PLUS_term;
  }
  // expr → expr MINUS term
  else if (Children[1].is("MINUS")) {
    return expr__expr_MINUS_term;
  }
  return notExpr;
}

TermRule Token::getTermRule() {
  if (Children.size() == 1) {
    // term → factor
    return term__factor;
  }
  // term → term STAR factor
  else if (Children[1].is("STAR")) {
    return term__term_STAR_factor;
  }
  // term → term SLASH factor
  else if (Children[1].is("SLASH")) {
    return term__term_SLASH_factor;
  }
  // term → term PCT factor
  else if (Children[1].is("PCT")) {
    return term__term_PCT_factor;
  }
  return notTerm;
}

LvalueRule Token::getLvalueRule() {
  // lvalue → ID
  if (Children.size() == 1) {
    return lvalue__ID;
  }
  // lvalue → STAR factor
  else if (Children.size() == 2) {
    return lvalue__STAR_factor;
  }
  // lvalue → LPAREN lvalue RPAREN
  else if (Children.size() == 3) {
    return lvalue__LPAREN_lvalue_RPAREN;
  }
  return notLvalue;
}

DclsRule Token::getDclsRule() {
  // dcls →
  if (Children.size() == 0) {
    return dcls__;
  }
  // dcls → dcls dcl BECOMES NUM SEMI
  else if (Children[3].is("NUM")) {
    return dcls__dcls_dcl_BECOMES_NUM_SEMI;
  }
  // dcls → dcls dcl BECOMES NULL SEMI
  else if (Children[3].is("NULL")) {
    return dcls__dcls_dcl_BECOMES_NULL_SEMI;
  }
  return notDcls;
}

StatementRule Token::getStatementRule() {
  if (Children.size() == 4) {
    return statement__lvalue_BECOMES_expr_SEMI;
  } else if (Children.size() == 5) {
    // statement → PRINTLN LPAREN expr RPAREN SEMI
    if (Children[0].is("PRINTLN")) {
      return statement__PRINTLN_LPAREN_expr_RPAREN_SEMI;
    }
    // statement → DELETE LBRACK RBRACK expr SEMI
    else {
      return statement__DELETE_LBRACK_RBRACK_expr_SEMI;
    }
  }
  // statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE
  else if (Children.size() == 7) {
    return statement__WHILE_LPAREN_test_RPAREN_LBRACE_statements_RBRACE;
  }
  // statement → IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE
  // statements RBRACE
  else {
    return statement__IF_LPAREN_test_RPAREN_LBRACE_statements_RBRACE_ELSE_LBRACE_statements_RBRACE;
  }
  return notStatement;
}

TestRule Token::getTestRule() {
  if (Children[1].is("EQ")) {
    return test__expr_EQ_expr;
  } else if (Children[1].is("NE")) {
    return test__expr_NE_expr;
  } else if (Children[1].is("LT")) {
    return test__expr_LT_expr;
  } else if (Children[1].is("LE")) {
    return test__expr_LE_expr;
  } else if (Children[1].is("GE")) {
    return test__expr_GE_expr;
  } else if (Children[1].is("GT")) {
    return test__expr_GT_expr;
  }
  return notTest;
}