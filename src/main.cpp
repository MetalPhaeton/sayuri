/* The MIT License (MIT)
 *
 * Copyright (c) 2015 Hironori Ishibashi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file main.cpp
 * @author Hironori Ishibashi
 * @brief メイン関数。
 */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <memory>
#include <fstream>

#include "sayuri.h"

// デバッグスイッチ。
// #define SAYURI_DEBUG_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

/**
 * UCI出力を標準出力に出力するコールバック関数。
 * @param message UCIShellからのメッセージ。
 */
void Print(const std::string& message) {
  std::cout << message << std::endl;
}

/**
 * 実行ループ。
 * @param shell 実行するUCIShell。
 */
void Run(Sayuri::UCIShell& shell) {
  while (true) {
    std::string input;
    std::getline(std::cin, input);

    if (input == "quit") {
      break;
    } else {
      shell.InputCommand(input);
    }
  }
}

/**
 * メイン関数。
 * @param argc コマンドの引数の数。
 * @param argv コマンドの引数。
 * @return 終了コード。
 */
int main(int argc, char* argv[]) {
#if defined(SAYURI_DEBUG_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063)
  // デバッグ用。
  return Sayuri::DebugMain(argc, argv);

#else
  if ((argc >= 2)
  && (std::strcmp(argv[1], "--help") == 0)) {
    // ヘルプの表示。
    std::string usage_str =
R"...(Usage:
    $ sayuri [option]

[option]:
    --version
        Shows version.

    --license
        Shows license terms.

    --help
        Shows help.

    --sayulisp <file name>
        Runs Sayuri as Sayulisp Interpreter.
        <file name> is Sayulisp script.
        If <file name> is '-', Sayuri reads script from standard input.)...";
    std::cout << usage_str << std::endl;
  } else if ((argc >= 2)
  && (std::strcmp(argv[1], "--version") == 0)) {
    // バージョン番号の表示。
    std::cout << Sayuri::ID_NAME << std::endl;
  } else if ((argc >= 2)
  && (std::strcmp(argv[1], "--license") == 0)) {
    // ライセンス分の表示。
    std::cout << Sayuri::LICENSE << std::endl;
  } else if ((argc >= 2)
  && (std::strcmp(argv[1], "--sayulisp") == 0)) {
    // Sayulispモード。
    // 引数がもう一つ必要。
    if (argc < 3) {
      std::cerr << "Insufficient arguments. '--sayulisp' needs <file name>."
      << std::endl;
      return 1;
    }

    // エンジン初期化。
    Sayuri::Init();

    // Sayurlispを作成。
    std::unique_ptr<Sayuri::Sayulisp>
    sayulisp_ptr(new Sayuri::Sayulisp());

    // 入力ストリームを得る。
    std::istream* stream_ptr = nullptr;
    std::ifstream file;
    if (std::strcmp(argv[2], "-") == 0) {
      stream_ptr = &(std::cin);
    } else {
      file.open(argv[2]);
      if (!file) {
        std::cerr << "Couldn't open '" << argv[2] << "'." << std::endl;
        return 1;
      }
      stream_ptr = &file;
    }

    // 実行。
    int status = 0;
    try {
      status = sayulisp_ptr->Run(stream_ptr);
    } catch (Sayuri::LispObjectPtr error) {
      return 1;
    }

    // 終了。
    if (file) file.close();
    return status;
  } else {
    // プログラムの起動。
    // 初期化。
    Sayuri::Init();

    // エンジン準備。
    std::unique_ptr<Sayuri::SearchParams>
    search_params_ptr(new Sayuri::SearchParams());

    std::unique_ptr<Sayuri::EvalParams>
    eval_params_ptr(new Sayuri::EvalParams());

    std::unique_ptr<Sayuri::TranspositionTable>
    table_ptr(new Sayuri::TranspositionTable(Sayuri::UCI_DEFAULT_TABLE_SIZE));

    std::unique_ptr<Sayuri::ChessEngine>
    engine_ptr(new Sayuri::ChessEngine(*search_params_ptr, *eval_params_ptr,
    *table_ptr));

    std::unique_ptr<Sayuri::UCIShell>
    shell_ptr(new Sayuri::UCIShell(*engine_ptr));

    shell_ptr->AddOutputListener(Print);

    // エンジン起動。
    Run(*shell_ptr);

    // 後処理。
    Sayuri::Postprocess();
  }

  return EXIT_SUCCESS;
#endif
}
