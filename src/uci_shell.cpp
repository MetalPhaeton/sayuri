/*
   uci_shell.cpp: UCIのチェスエンジン側インターフェイスの実装。

   The MIT License (MIT)

   Copyright (c) 2013 Ishibashi Hironori

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

#include "uci_shell.h"

#include <iostream>
#include "chess_def.h"
#include "chess_util.h"
#include "pv_line.h"
#include "error.h"

namespace Sayuri {
  /****************/
  /* static関数。 */
  /****************/
  // PV情報を標準出力に送る。
  void UCIShell::SendPVInfo(int depth, int seldepth, int score,
  Chrono::milliseconds time, std::size_t num_nodes, PVLine& pv_line) {
    std::cout << "info";
    std::cout << " depth " << depth;
    std::cout << " seldepth " << seldepth;
    std::cout << " score ";
    if (pv_line.line()[pv_line.length() - 1].has_checkmated()) {
      int winner = (pv_line.length() - 1) % 2;
      if (winner == 1) {
        // エンジンがメイトした。
        std::cout << "mate " << (pv_line.length() - 1);
      } else {
        // エンジンがメイトされた。
        std::cout << "mate " << (-1 * (pv_line.length() - 1));
      }
    } else {
      std::cout << "cp " << score;
    }
    std::cout << " time " << time.count();
    std::cout << " nodes " << num_nodes;
    // PVラインを送る。
    std::cout << " pv";
    Move move;
    char str[6];
    for (std::size_t i = 0; i < pv_line.length(); i++) {
      if (pv_line.line()[i].has_checkmated()) break;

      move = pv_line.line()[i].move();
      str[0] = Util::GetFyle(move.from_) + 'a';
      str[1] = Util::GetRank(move.from_) + '1';
      str[2] = Util::GetFyle(move.to_) + 'a';
      str[3] = Util::GetRank(move.to_) + '1';
      switch (move.promotion_) {
        case KNIGHT:
          str[4] = 'n';
          break;
        case BISHOP:
          str[4] = 'b';
          break;
        case ROOK:
          str[4] = 'r';
          break;
        case QUEEN:
          str[4] = 'q';
          break;
        default:
          str[4] = '\0';
          break;
      }
      str[5] = '\0';
      std::cout << " " << str;
    }
    std::cout << std::endl;
  }

  // 深さ情報を標準出力に送る。
  void UCIShell::SendDepthInfo(int depth) {
    std::cout << "info depth " << depth << std::endl;
  }

  // その他の情報を標準出力に送る。
  void UCIShell::SendOtherInfo(Chrono::milliseconds time,
  int hashfull, int nps) {
    std::cout << "info time " << time.count();
    std::cout << " hashfull " << hashfull;
    std::cout << " nps " << nps << std::endl;
  }
}  // namespace Sayuri
