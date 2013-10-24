#include <iostream>
#include "init.h"
#include "chess_engine.h"
#include "uci_shell.h"

int main(int argc, char* argv[]) {
  // 初期化。
  Sayuri::Init();

  // エンジン準備。
  std::unique_ptr<Sayuri::ChessEngine> engine_ptr(new Sayuri::ChessEngine());
  std::unique_ptr<Sayuri::UCIShell>
  shell_ptr(new Sayuri::UCIShell(engine_ptr.get()));

  // エンジン起動。
  shell_ptr->Run();

  return 0;
}
