#include <iostream>
#include <cstring>
#include <cstdlib>
#include "chess_def.h"
#include "init.h" #include "chess_engine.h"
#include "uci_shell.h"

int main(int argc, char* argv[]) {

  if ((argc >= 2)
  && (std::strcmp(argv[1], "--help") == 0)) {
    // ヘルプの表示。
    std::cout << "使い方:" << std::endl;
    std::cout << "\tsayuri [オプション]" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "オプション:" << std::endl;
    std::cout << "\t--version" << std::endl;
    std::cout << "\t\tバージョンを表示。" << std::endl;
    std::cout << "\t--help" << std::endl;
    std::cout << "\t\tヘルプを表示。" << std::endl;
    
  } else if ((argc >= 2)
  && (std::strcmp(argv[1], "--version") == 0)) {
    // バージョン番号の表示。
    std::cout << Sayuri::ID_NAME << std::endl;
  } else {
    // プログラムの起動。
    // 初期化。
    Sayuri::Init();

    // エンジン準備。
    std::unique_ptr<Sayuri::ChessEngine> engine_ptr(new Sayuri::ChessEngine());
    std::unique_ptr<Sayuri::UCIShell>
    shell_ptr(new Sayuri::UCIShell(engine_ptr.get()));

    // エンジン起動。
    shell_ptr->Run();
  }

  return EXIT_SUCCESS;
}
