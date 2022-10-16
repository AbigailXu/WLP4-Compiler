#include "Structure.h"
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

Structure::Structure(std::istream &cinput) : cinput{cinput} {}
int variableCounter = 0;
int labelCounter = 0;

// =============================== build tree ===============================
void Structure::addChildren(std::istream &in, Token &parent) {
  string line;
  if (getline(in, line)) {
    istringstream iss(line);

    // get parent name
    string parent_name;
    iss >> parent_name;

    // make children; add to parent
    string cur_child_name;
    while (iss >> cur_child_name) {
      Token cur_child = Token(cur_child_name);
      parent.Children.push_back(cur_child);
    }

    // loop children; addChildren(in, child_ptr)
    for (auto &cur_child : parent.Children) {
      if (!cur_child.isTerminal()) {
        addChildren(in, cur_child);
      } else {
        getline(in, line);
        istringstream iss_terminal(line);
        string sym;
        string contex;
        iss_terminal >> sym >> contex;
        cur_child.Contex = contex;
        // cout << cur_child.Symbol << endl;
      }
    }
  }
}

// =============================== get params ==============================
void Structure::getParams(Token &paramlist, std::string procedure_name) {
  string cur_param_name = paramlist.Children[0].getDclId();
  all_procedures.at(procedure_name)
      .first.emplace_back(
          make_pair(paramlist.Children[0].getDclType(), cur_param_name));
  if (paramlist.getChildCount() == 3) {
    getParams(paramlist.Children[2], procedure_name);
  }
}

// =============================== get procedure ==============================
void Structure::getProcedureSign(Token &procedure_root) {
  string procedure_name = procedure_root.getProcedureName();
  vector<pair<Type, string>> procedure_params;

  map<string, pair<Type, int>> cur_symbol_table;
  // add params for "wain"
  if (procedure_root.is("main")) {
    for (auto &cur_child : procedure_root.Children) {
      if (cur_child.is("dcl")) {
        procedure_params.push_back(
            make_pair(cur_child.getDclType(), cur_child.getDclId()));
      }
    }
    // add procedure

    all_procedures.insert(
        {procedure_name, make_pair(procedure_params, cur_symbol_table)});
  }
  // add params for "other procedures"
  else {
    // add procedure, name only
    all_procedures.insert(
        {procedure_name, make_pair(procedure_params, cur_symbol_table)});
    // add params
    if (procedure_root.Children[3].getChildCount() > 0) {
      Token &paramlist = procedure_root.Children[3].Children[0];
      getParams(paramlist, procedure_name);
    }
  }
}

// ========================== get variable table ==========================
void Structure::getVariable(Token &parent, std::string procedure_name) {
  // add Variable when read "dcl"
  if (parent.is("dcl")) {
    string cur_dcl_Id = parent.getDclId();
    Type cur_dcl_type = parent.getDclType();
    all_procedures.at(procedure_name)
        .second.insert({cur_dcl_Id, make_pair(cur_dcl_type, variableCounter)});
    variableCounter -= 4;
  }
  // visit children
  for (auto &cur_child : parent.Children) {
    if (!cur_child.isTerminal()) {
      getVariable(cur_child, procedure_name);
    }
  }
}

// ========================  get all procedure ===========================
void Structure::getAllProcedures(Token &parent) {
  getProcedureSign(parent.Children[0]);
  if (parent.Children[0].is("procedure") || parent.Children[0].is("main")) {
    string cur_procedure_name = parent.Children[0].getProcedureName();
    variableCounter = 0;
    getVariable(parent.Children[0], cur_procedure_name);
  }
  if (parent.is("procedures") && parent.getChildCount() > 1) {
    getAllProcedures(parent.Children[1]);
  }
}

// ======================== is Param ? ===========================

bool Structure::isParam(string procedure_name, string variable_name) {
  for (auto &param : all_procedures.at(procedure_name).first) {
    string cur_param_name = param.second;
    if (variable_name.compare(cur_param_name) == 0) {
      return true;
    }
    // cerr <<  1 << endl;
  }
  return false;
}

// ======================== get Param count ===========================

int Structure::getParamCount(string procedure_name, string variable_name) {
  return all_procedures.at(procedure_name).first.size();
}

// ========================= get  variable offset =============================
int Structure::getVariableOffset(string variable_name, string procedure_name) {
  for (auto &var : all_procedures.at(procedure_name).second) {
    if (variable_name.compare(var.first) == 0) {
      return var.second.second;
    }
  }
  return -1;
}

void Structure::setVariableOffset(string variable_name, string procedure_name,
                                  int offset) {
  for (auto &var : all_procedures.at(procedure_name).second) {
    if (variable_name.compare(var.first) == 0) {
      var.second.second = offset;
      break;
    }
  }
}
// ========================= get current variable offset
// =============================

int Structure::getCurrentOffset(string variable_name, string procedure_name) {
  int variable_offset = getVariableOffset(variable_name, procedure_name);
  // is not main
  if (procedure_name.compare("wain") != 0) {
    int param_count = getParamCount(procedure_name, variable_name);
    variable_offset = variable_offset + (param_count * 4);
  }
  return variable_offset;
}

void comment(string content) { cout << " ; " << content << endl; }
string label(string name) {
  string cur_label = name + to_string(labelCounter);
  ++labelCounter;
  return cur_label;
}

Token &Structure::getMain(Token &parent) {
  if (parent.is("procedures")) {
    for (auto &child : parent.Children) {
      if (child.is("main")) {
        return child;
      } else if (child.is("procedures")) {
        return getMain(child);
      }
    }
  } else {
    cout << "; cannot get Main" << endl;
  }
}

// ======================== push() ===============================
void Structure::push(string reg) {
  comment("push " + reg);
  cout << "sw " << reg << ", -4($30)";
  comment(reg + " -> stack");
  cout << "sub $30, $30, $4";
  comment("stack pointer up");
}

// ======================== pop() ===============================
void Structure::pop(string reg) {
  cout << "add $30, $30, $4";
  comment("stack pointer down");
  cout << "lw " << reg << ", -4($30)";
  comment("stack -> " + reg);
}

// ======================== print $1 ===========================
void Structure::printMips(string reg) {
  push("$1");
  cout << "add $1, $0, " << reg << endl;
  push("$31");
  cout << "lis $5" << endl;
  cout << ".word print" << endl;
  cout << "jalr $5" << endl;
  pop("$31");
  pop("$1");
}

// ======================= call function ========================
void Structure::callFunction(string functionName) {
  comment("------ call " + functionName + " ------");
  push("$31");
  push("$5");
  cout << "lis $5" << endl;
  cout << ".word " << functionName << endl;
  cout << "jalr $5" << endl;
  pop("$5");
  pop("$31");
  comment("----------------------------");
}

// -======================= get Type =============================
Type Structure::getType(Token &parent, string procedure_name) {
  // NUM
  if (parent.is("NUM")) {
    return INT;
  }
  // ID
  else if (parent.is("ID")) {
    string IdName = parent.Contex;
    // is procedure
    if (all_procedures.find(IdName) != all_procedures.end()) {
      return INT;
    }
    // is variable
    else {
      Type variable_type =
          all_procedures.at(procedure_name).second.at(parent.Contex).first;
      return variable_type;
    }
  }
  // NULL
  else if (parent.is("NULL")) {
    return PTR;
  }
  // factor
  else if (parent.is("factor")) {
    switch (parent.getFactorRule()) {
    case factor__ID:
      return getType(parent.Children[0], procedure_name);
    case factor__NUM:
      return INT;
    case factor__NULL:
      return PTR;
    case factor__AMP_lvalue:
      return PTR;
    case factor__STAR_factor:
      return INT;
    case factor__LPAREN_expr_RPAREN:
      return getType(parent.Children[1], procedure_name);
    case factor__ID_LPAREN_RPAREN:
      return INT;
    case factor__NEW_INT_LBRACK_expr_RBRACK:
      return PTR;
    case factor__ID_LPAREN_arglist_RPAREN: {
      Token *cur_arglist = &parent.Children[2];
      int arg_idx = 0;
      string called_procedure_name = parent.Children[0].Contex;
      while (true) {
        Type cur_arg_type = getType(cur_arglist->Children[0], procedure_name);
        Type cur_param_type =
            all_procedures.at(called_procedure_name).first[arg_idx].first;
        // children
        if (cur_arglist->getChildCount() > 1) {
          ++arg_idx;
          cur_arglist = &(cur_arglist->Children[2]);
        } else {
          break;
        }
      }
      return INT;
    }
    default:
      cerr << "ERROR: cannot find matching rule for factor" << endl;
      break;
    }
  }
  // expr
  else if (parent.is("expr")) {
    // cerr << ">  expr" << endl;
    switch (parent.getExprRule()) {
    case expr__term:
      return getType(parent.Children[0], procedure_name);
    case expr__expr_PLUS_term: {
      // cerr << "   >  expr PLUS term" << endl;
      Type t1 = getType(parent.Children[0], procedure_name);
      Type t2 = getType(parent.Children[2], procedure_name);
      if (t1 == INT && t2 == INT) {
        return INT;
      } else if (t1 == PTR && t2 == INT) {
        return PTR;
      } else if (t1 == INT && t2 == PTR) {
        return PTR;
      }
      cerr << "ERROR: type error during addition" << endl;
      return err;
    }
    case expr__expr_MINUS_term:
      if (getType(parent.Children[0], procedure_name) == INT &&
          getType(parent.Children[2], procedure_name) == INT) {
        return INT;
      } else if (getType(parent.Children[0], procedure_name) == PTR &&
                 getType(parent.Children[2], procedure_name) == INT) {
        return PTR;
      } else if (getType(parent.Children[0], procedure_name) == PTR &&
                 getType(parent.Children[2], procedure_name) == PTR) {
        return INT;
      }
      cerr << "ERROR: type error during subtraction" << endl;
      break;
    default:
      cerr << "ERROR: cannot find matching rule for expr" << endl;
      break;
    }
  }
  // term
  else if (parent.is("term")) {
    // cerr << ">  term" << endl;
    switch (parent.getTermRule()) {
    case term__factor:
      return getType(parent.Children[0], procedure_name);
    case term__term_STAR_factor:
      return INT;
    case term__term_SLASH_factor:
      return INT;
    case term__term_PCT_factor: {
      return INT;
    }
    default:
      cerr << "ERROR: cannot find matching rule for term" << endl;
      break;
    }
  }
  // lvalue
  else if (parent.is("lvalue")) {
    // cerr << ">  lvalue" << endl;
    switch (parent.getLvalueRule()) {
    case lvalue__ID: {
      Type idType = all_procedures.at(procedure_name)
                        .second.at(parent.Children[0].Contex)
                        .first;
      return idType;
    }
    case lvalue__STAR_factor:
      return INT;
    case lvalue__LPAREN_lvalue_RPAREN:
      return getType(parent.Children[1], procedure_name);
    default:
      cerr << "ERROR: cannot find matching rule for lvalue" << endl;
    }
  } else {
    for (auto &child : parent.Children) {
      if (!child.isTerminal()) {
        getType(child, procedure_name);
      }
    }
  }
}

// ======================== print Prologue main =============================
void Structure::print_Prologue_Main(Token &parent, string procedure_name) {
  cout << "; begin prologue" << endl;

  // import
  cout << ".import print" << endl;
  cout << ".import init" << endl;
  cout << ".import new" << endl;
  cout << ".import delete" << endl;
  // setup
  cout << "lis $4" << endl;
  cout << ".word 4" << endl;
  cout << "lis $10" << endl;
  cout << ".word print" << endl;
  cout << "lis $11" << endl;
  cout << ".word 1" << endl;

  // if param 1 is PTR
  if (parent.Children[3].getDclType() == PTR) {
    callFunction("init"); // init memory
  } else {
    push("$2");
    cout << "add $2, $0, $0" << endl;
    callFunction("init"); // init memory
    pop("$2");
  }

  // frame pointer
  cout << "sub $29, $30, $4" << endl;

  push("$1");
  push("$2");

  // dcls
  code(parent.Children[8], procedure_name);

  cout << "; end prologue" << endl;
}

// ======================== print Epilogue main =============================
void Structure::print_Epilogue_Main(Token &parent, string procedure_name) {
  map<string, pair<Type, int>> &variables =
      all_procedures.at(procedure_name).second;
  cout << "; begin eppilogue" << endl;
  for (auto &symbol : variables) {
    cout << "add $30, $30, $4" << endl; // for variable count number of times
  }
  cout << "jr $31" << endl;
}
// ======================== print Prologue others =============================
void Structure::print_Prologue_Others(Token &parent, string procedure_name) {
  cout << "; begin prologue" << endl;
  cout << "F" << procedure_name << ":" << endl;

  cout << "sub $29, $30, $4" << endl;

  // dcls
  code(parent.Children[6], procedure_name);
  // save regesters ??
  push("$5");
  push("$6");
  push("$7");

  cout << "; end prologue" << endl;
}

// ======================== print Epilogue others =============================
void Structure::print_Epilogue_Other(Token &parent, string procedure_name) {
  map<string, pair<Type, int>> &variables =
      all_procedures.at(procedure_name).second;
  cout << "; begin eppilogue" << endl;

  // pop regesters ??
  pop("$7");
  pop("$6");
  pop("$5");
  for (auto &symbol : variables) {
    cout << "add $30, $30, $4" << endl; // for variable count number of times
  }

  cout << "add $30, $29, $4" << endl;
  cout << "jr $31" << endl;
}

// ======================= * main generator * =========================

Type Structure::code(Token &parent, string procedure_name) {
  // NUM
  if (parent.is("NUM")) {
    cout << "lis $3" << endl;
    cout << ".word " << parent.Contex << endl;
    return INT;
  }
  // ID
  else if (parent.is("ID")) {
    string IdName = parent.Contex;
    // is procedure
    if (all_procedures.find(IdName) != all_procedures.end()) {
      return INT;
    }
    // is variable
    else {
      int variable_offset = getCurrentOffset(IdName, procedure_name);
      cout << "lw $3, " << variable_offset << "($29)";
      cout << " ; $3 <- " << parent.Contex << endl;
    }
  }
  // NULL
  else if (parent.is("NULL")) {
    return PTR;
  }
  // factor
  else if (parent.is("factor")) {
    // cerr << ">  factor" << endl;
    switch (parent.getFactorRule()) {
    case factor__ID:
      return code(parent.Children[0], procedure_name);
    case factor__NUM:
      return code(parent.Children[0], procedure_name);
    case factor__NULL:
      cout << "add $3, $0, $11" << endl;
      comment("NULL");
      return PTR;
    case factor__AMP_lvalue: {
      Token &lvalue = parent.Children[1];
      while (true) {
        if (lvalue.getLvalueRule() == lvalue__ID) {
          int variable_offset =
              getCurrentOffset(lvalue.Children[0].Contex, procedure_name);
          cout << "lis $3" << endl;
          cout << ".word " << variable_offset;
          comment("&" + lvalue.Children[0].Contex);
          cout << "add $3, $3, $29" << endl;
          break;
        } else if (lvalue.getLvalueRule() == lvalue__STAR_factor) {
          code(lvalue.Children[1], procedure_name);
          break;
        } else if (lvalue.getLvalueRule() == lvalue__LPAREN_lvalue_RPAREN) {
          lvalue = lvalue.Children[1];
        }
      }

      return PTR;
    }
    case factor__STAR_factor:
      code(parent.Children[1], procedure_name);
      cout << "lw $3, 0($3)";
      comment("$3 = * $3");
      return INT;
    case factor__LPAREN_expr_RPAREN:
      return code(parent.Children[1], procedure_name);
    case factor__ID_LPAREN_RPAREN:
      comment("********* call " + parent.Children[0].Contex + " *********");
      push("$29");
      push("$31");
      cout << "lis $5" << endl;
      cout << ".word "
           << "F" << parent.Children[0].Contex << endl;
      cout << "jalr $5" << endl;
      pop("$31");
      pop("$29");
      comment("********* call " + parent.Children[0].Contex + " (end)" +
              " *********");
      return INT;
    case factor__NEW_INT_LBRACK_expr_RBRACK: {
      comment("-------------- NEW --------------");
      push("$1");
      Token &expr = parent.Children[3];
      code(expr, procedure_name);
      cout << "add $1, $3, $0";
      comment("NEW procedure expects value in $1");
      callFunction("new");
      cout << "bne $3, $0, 1";
      comment("if called, skip next line");
      cout << "add $3, $11, $0";
      comment("set $3 to NULL if allocation fails");
      pop("$1");
      comment("---------------------------------");
      return PTR;
    }
    case factor__ID_LPAREN_arglist_RPAREN: {
      string called_procedure_name = parent.Children[0].Contex;
      comment("********* call " + called_procedure_name + " *********");
      push("$29");
      push("$31");
      Token *cur_arglist = &parent.Children[2];
      int arg_idx = 0;
      while (true) {
        comment("push param " + to_string(arg_idx));
        code(cur_arglist->Children[0], procedure_name);
        push("$3");
        ++arg_idx;
        // children
        if (cur_arglist->getChildCount() > 1) {
          cur_arglist = &(cur_arglist->Children[2]);
        } else {
          break;
        }
      }
      cout << "lis $5" << endl;
      cout << ".word "
           << "F" << called_procedure_name << endl;
      cout << "jalr $5" << endl;
      for (int i = 0; i < arg_idx; ++i) {
        comment("pop param " + to_string(arg_idx - i));
        pop("$31");
      }
      pop("$31");
      pop("$29");
      comment("********* call " + called_procedure_name + " (end)" +
              " *********");
      return INT;
    }
    default:
      cerr << "ERROR: cannot find matching rule for factor" << endl;
      break;
    }
  }
  // expr
  else if (parent.is("expr")) {
    // cout << ">  expr" << endl;
    switch (parent.getExprRule()) {
    case expr__term:
      return code(parent.Children[0], procedure_name);

    case expr__expr_PLUS_term: {
      Type t1 = getType(parent.Children[0], procedure_name);
      Type t2 = getType(parent.Children[2], procedure_name);

      // ptr + int
      if (t1 == PTR) {
        code(parent.Children[0], procedure_name);
        push("$3");
        code(parent.Children[2], procedure_name);
        cout << "mult $3, $4";
        comment("$4 always has value 4");
        cout << "mflo $3" << endl;
        pop("$5");
        cout << "add $3, $5, $3" << endl;
      } else if (t2 == PTR) {
        code(parent.Children[2], procedure_name);
        push("$3");
        code(parent.Children[0], procedure_name);
        cout << "mult $3, $4";
        comment("$4 always has value 4");
        cout << "mflo $3" << endl;
        pop("$5");
        cout << "add $3, $5, $3" << endl;
      } else {
        code(parent.Children[0], procedure_name);
        push("$3");
        code(parent.Children[2], procedure_name);
        pop("$5");
        cout << "add $3, $5, $3";
        comment("$3 = $5 + $3");
      }
      break;
    }

    case expr__expr_MINUS_term: {
      Type t1 = getType(parent.Children[0], procedure_name);
      Type t2 = getType(parent.Children[2], procedure_name);
      comment("|----------- sub -------------");
      // ptr - int
      if (t1 == PTR && t2 == INT) {
        code(parent.Children[0], procedure_name);
        push("$3");
        code(parent.Children[2], procedure_name);
        cout << "mult $3, $4";
        comment("$4 always has value 4");
        cout << "mflo $3" << endl;
        pop("$5");
        cout << "sub $3, $5, $3" << endl;
      } else if (t1 == PTR && t2 == PTR) {
        code(parent.Children[0], procedure_name);
        push("$3");
        code(parent.Children[2], procedure_name);
        pop("$5");
        cout << "sub $3, $5, $3" << endl;
        cout << "div $3, $4" << endl;
        cout << "mflo $3" << endl;
      } else {
        code(parent.Children[0], procedure_name);
        push("$3");
        code(parent.Children[2], procedure_name);
        pop("$5");
        cout << "sub $3, $5, $3";
        comment("$3 = $5 - $3");
      }
      comment("----------- sub -------------|");
      break;
    }
    default:
      break;
    }
  }
  // term
  else if (parent.is("term")) {
    // cerr << ">  term" << endl;
    switch (parent.getTermRule()) {

    case term__factor:
      return code(parent.Children[0], procedure_name);

    case term__term_STAR_factor:
      code(parent.Children[0], procedure_name);
      push("$3");
      code(parent.Children[2], procedure_name);
      pop("$5");
      cout << "mult $5, $3" << endl;
      cout << "mflo $3";
      comment("$3 = $5 * $3");
      return INT;

    case term__term_SLASH_factor:
      code(parent.Children[0], procedure_name);
      push("$3");
      code(parent.Children[2], procedure_name);
      pop("$5");
      cout << "div $5, $3" << endl;
      cout << "mflo $3";
      comment("$3 = $5 / $3");
      return INT;

    case term__term_PCT_factor:
      code(parent.Children[0], procedure_name);
      push("$3");
      code(parent.Children[2], procedure_name);
      pop("$5");
      cout << "div $5, $3" << endl;
      cout << "mfhi $3";
      comment("$3 = $5 / $3");
      return INT;

    default:
      cerr << "ERROR: cannot find matching rule for term" << endl;
      break;
    }
  }
  // lvalue
  else if (parent.is("lvalue")) {
    switch (parent.getLvalueRule()) {
    case lvalue__ID:
      code(parent.Children[0], procedure_name);
    case lvalue__STAR_factor: {
      comment("------------- lvalue ---------------");
      code(parent.Children[1], procedure_name);
      cout << "lw $3, 0($3)";
      comment("$3 = * $3");
      // Token &factor = parent.Children[1];
      // int offset = getVariableOffset(factor.Children[0].Contex);
      // cout << "lis $3" << endl;
      // cout << ".word " << offset;
      // comment("$3 = offset");
      comment("-----------------------------------");
      return INT;
    }
    case lvalue__LPAREN_lvalue_RPAREN: // not sure
      return code(parent.Children[1], procedure_name);
    default:
      cerr << "ERROR: cannot find matching rule for lvalue" << endl;
    }
  }
  // statements
  else if (parent.is("statements")) {
    if (parent.getChildCount() > 0) {
      code(parent.Children[0], procedure_name);
      code(parent.Children[1], procedure_name);
    }
  }
  // statement
  else if (parent.is("statement")) {
    switch (parent.getStatementRule()) {
    case statement__lvalue_BECOMES_expr_SEMI: {
      comment("||-------- become ----------");
      Token &lvalue = parent.Children[0];
      Token &expr = parent.Children[2];
      while (true) { // handle ()
        // lvalue -> ID
        if (lvalue.getLvalueRule() == lvalue__ID) {
          int variable_offset = getCurrentOffset(lvalue.Children[0].Contex, procedure_name);
          code(expr, procedure_name);
          cout << "sw $3, " << variable_offset << "($29)" << endl;
          comment("-------- become ----------||");
          break;
        }
        // lvalue -> STAR factor
        else if (lvalue.getLvalueRule() == lvalue__STAR_factor) {
          code(expr, procedure_name);
          push("$3");
          Token &factor = lvalue.Children[1];
          code(factor, procedure_name);
          pop("$5");
          cout << "sw $5, 0($3)";
          comment("lvalue => STAR factor");
          comment("-------- become ----------||");
          break;
        }
        // lvalue -> ( lvalue )
        else if (lvalue.getLvalueRule() == lvalue__LPAREN_lvalue_RPAREN) {
          lvalue = lvalue.Children[1];
          comment(":(");
        }
      }
      comment("-------- become ----------||");
      break;
    }
    case statement__IF_LPAREN_test_RPAREN_LBRACE_statements_RBRACE_ELSE_LBRACE_statements_RBRACE: {
      string else_label = label("else");
      string endif_label = label("endif");
      code(parent.Children[2], procedure_name);
      cout << "beq $3, $0, " << else_label << endl;
      code(parent.Children[5], procedure_name);
      cout << "beq $0, $0, " << endif_label << endl;
      cout << else_label << ":" << endl;
      code(parent.Children[9], procedure_name);
      cout << endif_label << ":" << endl;
      break;
    }
    case statement__WHILE_LPAREN_test_RPAREN_LBRACE_statements_RBRACE: {
      string loop_label = label("loop");
      string endWhile_label = label("endWhile");
      cout << loop_label << ":" << endl;
      code(parent.Children[2], procedure_name);
      cout << "beq $3, $0, " << endWhile_label << endl;
      code(parent.Children[5], procedure_name);
      cout << "beq $0, $0, " << loop_label << endl;
      cout << endWhile_label << ":" << endl;
      break;
    }
    case statement__PRINTLN_LPAREN_expr_RPAREN_SEMI:
      comment("--------- print ---------");
      push("$1");
      code(parent.Children[2], procedure_name);
      cout << "add $1, $3, $0" << endl;
      push("$31");
      cout << "lis $5" << endl;
      cout << ".word print" << endl;
      cout << "jalr $5" << endl;
      pop("$31");
      pop("$1");
      comment("-------------------------");
      break;
    case statement__DELETE_LBRACK_RBRACK_expr_SEMI: {
      Token &expr = parent.Children[3];
      string skipDelete_label = label("skipDelete");
      code(expr, procedure_name);
      cout << "beq $3, $11, " << skipDelete_label << endl;
      cout << "add $1, $3, $0" << endl;
      push("$31");
      cout << "lis $5" << endl;
      cout << ".word delete" << endl;
      cout << "jalr $5" << endl;
      pop("$31");
      cout << skipDelete_label << ":" << endl;
      break;
    }
    default:
      cerr << "ERROR: cannot find matching rule for statement" << endl;
      break;
    }
  }
  // dcls
  else if (parent.is("dcls")) {
    switch (parent.getDclsRule()) {
    case dcls__:
      break;

    case dcls__dcls_dcl_BECOMES_NUM_SEMI:
      code(parent.Children[0], procedure_name);
      cout << "lis $3";
      comment("dcl");
      cout << ".word " << parent.Children[3].Contex << endl;
      push("$3");
      break;

    case dcls__dcls_dcl_BECOMES_NULL_SEMI:
      code(parent.Children[0], procedure_name);
      cout << "add $5, $0, $11" << endl;
      push("$5");

      break;
    default:
      break;
    }
  }
  // test
  else if (parent.is("test")) {
    string slt_sltu = "slt";
    if (getType(parent.Children[0], procedure_name) == PTR) {
      slt_sltu = "sltu";
    }
    code(parent.Children[0], procedure_name);
    push("$3");
    code(parent.Children[2], procedure_name);
    pop("$5");
    switch (parent.getTestRule()) {
    case test__expr_EQ_expr:
      cout << slt_sltu << " $6, $3, $5" << endl;
      cout << slt_sltu << " $7, $5, $3" << endl;
      cout << "add $3, $6, $7" << endl;
      cout << "sub $3, $11, $3";
      comment("$3 = ($3 == $5)");
      break;
    case test__expr_NE_expr:
      cout << slt_sltu << " $6, $3, $5" << endl;
      cout << slt_sltu << " $7, $5, $3" << endl;
      cout << "add $3, $6, $7";
      comment("$3 = ($3 != $5)");
      break;
    case test__expr_LT_expr:
      cout << slt_sltu << " $3, $5, $3";
      comment("$3 = ($5 < $3)");
      break;
    case test__expr_LE_expr:
      cout << slt_sltu << " $3, $3, $5" << endl;
      cout << "sub $3, $11, $3";
      comment("$3 = ($3 <= $5)");
      break;
    case test__expr_GE_expr:
      cout << slt_sltu << " $3, $5, $3" << endl;
      cout << "sub $3, $11, $3";
      comment("$3 = ($5 >= $3)");
      break;
    case test__expr_GT_expr:
      cout << slt_sltu << " $3, $3, $5";
      comment("$3 = ($5 > $3)");
      break;
    }
  }
  // wain
  else if (parent.is("main")) {
    comment("============ " + procedure_name + " (start) ============");
    print_Prologue_Main(parent, procedure_name);
    code(parent.Children[9], procedure_name);  // statements
    code(parent.Children[11], procedure_name); // return expr
    print_Epilogue_Main(parent, procedure_name);
    comment("============ " + procedure_name + " (end) ==============");
  }
  // procedure
  else if (parent.is("procedure")) {
    string cur_procedure_name = parent.getProcedureName();
    comment("============ " + cur_procedure_name + " (start) ============");
    print_Prologue_Others(parent, cur_procedure_name);
    code(parent.Children[7], cur_procedure_name); // statements
    comment("RETURN");
    code(parent.Children[9], cur_procedure_name); // return expr
    print_Epilogue_Other(parent, cur_procedure_name);
    comment("============ " + cur_procedure_name + " (end) ==============");
  }
  // all procedures
  else if (parent.is("procedures")) {
    // only call main
    if (procedure_name.compare("main") == 0) {
      if (parent.getChildCount() == 1) {
        code(parent.Children[0], "wain");
      } else {
        code(parent.Children[1], "main");
      }
    }
    // only call others
    else {
      if (parent.getChildCount() > 1) {
        code(parent.Children[0], parent.Children[0].getProcedureName());
        code(parent.Children[1], "");
      }
    }
  } else {
    for (auto &child : parent.Children) {
      if (!child.isTerminal()) {
        code(child, procedure_name);
      }
    }
  }
}

// ===================================================================================
// ===================================================================================

void Structure::init() {
  string line;

  // build Tree
  addChildren(cinput, root);

  // get each procedure
  Token &all_proceduresRoot = root.Children[1];
  getAllProcedures(all_proceduresRoot);

  for (auto &cur_procedure : all_procedures) {

    // print signiture
    cerr << cur_procedure.first << ":";
    for (auto &cur_param : cur_procedure.second.first) {
      cerr << " " << getTypeString(cur_param.first);
    }
    cerr << endl;

    // print variables
    for (auto &symbol : cur_procedure.second.second) {
      cerr << symbol.first << "  " << getTypeString(symbol.second.first);
      if (symbol.second.first == PTR) {
        cerr << "  ";
      } else {
        cerr << "   ";
      }
      cerr << symbol.second.second << endl;
    }
  }
}

// ===================================================================================
// ===================================================================================
