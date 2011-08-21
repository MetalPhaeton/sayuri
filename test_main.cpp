/* test_main.cpp: テスト用プログラムのメイン。
   copyright (c) 2011 石橋宏之利
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "misaki.h"

#include "misaki_debug.h"

using namespace Misaki;

// テスト用タイトルをプリントする。
void PrintTitle() {
  std::cout << "************" << std::endl;
  std::cout << "* Test Run *" << std::endl;
  std::cout << "************" << std::endl;
}

int main(int argc, char* argv[]) {
  // テスト用タイトルをプリント。
  PrintTitle();

  Init();

  ChessBoard* board = ChessBoard::New();
  board->Test();
  delete board;

  return 0;
}
