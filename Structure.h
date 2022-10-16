#ifndef __structure__
#define __structure__
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

class Structure {
  int getReturnOffsetHelper(Token &parent);

public:
  // Table
  std::map<std::string, std::pair<std::vector<std::pair<Type, std::string>>,
                                  std::map<std::string, std::pair<Type, int>>>>
      all_procedures;
  // Tree
  Token root = Token("start", "");

  // other fields
  std::istream &cinput;

  Structure(std::istream &cinput);
  ~Structure(){};

  void addChildren(std::istream &in, Token &parent);
  void getParams(Token &paramlist, std::string procedure_name);
  void getProcedureSign(Token &procedure_root);
  void getVariable(Token &parent, std::string procedure_name);
  void getAllProcedures(Token &parent);
  Token &getMain(Token &parent);

  bool isParam(std::string procedure_name, std::string variable_name);
  int getParamCount(std::string procedure_name, std::string variable_name);
  int getVariableOffset(std::string variable_name, std::string procedure_name);
  void setVariableOffset(std::string variable_name, std::string procedure_name, int offset);
  int getCurrentOffset(std::string variable_name, std::string procedure_name);

  void printMips(std::string reg);
  void callFunction(std::string functionName);
  void print_Prologue_Main(Token &parent, std::string procedure_name);
  void print_Prologue_Others(Token &parent, std::string procedure_name);
  void print_Epilogue_Main(Token &parent, std::string procedure_name);
  void print_Epilogue_Other(Token &parent, std::string procedure_name);

  Type code(Token &parent, std::string procedure_name);
  Type getType(Token &parent, std::string procedure_name);
  void code(Token &item);
  void push(std::string reg);
  void pop(std::string reg);

  void init();
};

#endif