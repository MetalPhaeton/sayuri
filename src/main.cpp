/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
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

#include "sayuri.h"

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
    std::unique_ptr<Sayuri::SearchParams>
    search_params_ptr(new Sayuri::SearchParams());

    std::unique_ptr<Sayuri::EvalParams>
    eval_params_ptr(new Sayuri::EvalParams());

    std::unique_ptr<Sayuri::ChessEngine>
    engine_ptr(new Sayuri::ChessEngine(*search_params_ptr, *eval_params_ptr));

    std::unique_ptr<Sayuri::UCIShell>
    shell_ptr(new Sayuri::UCIShell(*engine_ptr));

    shell_ptr->AddOutputListener(Print);

    // エンジン起動。
    Run(*shell_ptr);

    // 後処理。
    Sayuri::Postprocess();
  }

  return EXIT_SUCCESS;
  // return Sayuri::DebugMain(argc, argv);
}
