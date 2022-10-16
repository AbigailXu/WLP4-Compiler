#include "Structure.h"
#include "Token.h"
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
using namespace std;


int main() {
  Structure structure = Structure(cin);
  structure.init();

  // call wain


  // call others
  Token & procedures = structure.root.Children[1];
  // cout <<"; " << structure.getMain(procedures).Symbol << endl;


  structure.code(structure.root, "main");
  structure.code(structure.root, "");

  // for (auto &cur_procedure : structure.all_procedures) {
  //   if (cur_procedure.first.compare("wain") != 0) {
  //     cout << "; START " << cur_procedure.first << endl;
  // //     structure.code(structure.root, cur_procedure.first);
  //   }
  // }

  return 0;
}