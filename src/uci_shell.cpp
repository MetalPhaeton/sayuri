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
#include <string>
#include <vector>
#include "chess_def.h"
#include "chess_util.h"
#include "pv_line.h"
#include "error.h"

#include "mylib.h"  // テスト用。

namespace Sayuri {
  /**************/
  /* テスト用。 */
  /**************/
  // =================================================================
  extern void PrintPosition
  (const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]);
  void UCIShell::Test() {
    std::string command = "position fen ";
    command +=
    "4k3/8/8/8/8/8/1p6/2N1K3 b - - 1 1";
    command += " moves b2c1q e1f2";

    std::cout << "Command: " << command << std::endl;

    std::vector<std::string> argv =
    MyLib::Split(command, std::string(" ") , std::string(""));

    CommandPosition(argv);
    PrintPosition(engine_ptr_->position());
  }
  // =================================================================

  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  UCIShell::UCIShell(ChessEngine* engine_ptr) :
  engine_ptr_(engine_ptr) {
  }

  // コピーコンストラクタ。
  UCIShell::UCIShell(const UCIShell& shell) :
  engine_ptr_(shell.engine_ptr_) {
  }

  // ムーブコンストラクタ。
  UCIShell::UCIShell(UCIShell&& shell) :
  engine_ptr_(shell.engine_ptr_) {
  }

  // コピー代入。
  UCIShell& UCIShell::operator=(const UCIShell& shell) {
    engine_ptr_ = shell.engine_ptr_;
    return *this;
  }

  // ムーブ代入。
  UCIShell& UCIShell::operator=(UCIShell&& shell) {
    engine_ptr_ = shell.engine_ptr_;
    return *this;
  }

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

  // 現在探索している手の情報を標準出力に送る。
  void UCIShell::SendCurrentMoveInfo(Move move, int move_num) {
    // 手の情報を送る。
    std::cout << "info currmove ";
    char str[6];
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
    std::cout << str;

    // 手の番号を送る。
    std::cout << " currmovenumber " << move_num << std::endl;
  }

  // その他の情報を標準出力に送る。
  void UCIShell::SendOtherInfo(Chrono::milliseconds time,
  std::size_t num_nodes, int hashfull) {
    int time_2 = time.count();
    if (time_2 <= 0) time_2 = 1;
    std::cout << "info time " << time_2;
    std::cout << " nodes " << num_nodes;
    std::cout << " hashfull " << hashfull;
    std::cout << " nps " << (num_nodes * 1000) / time_2 << std::endl;
  }

  /*********************/
  /* UCIコマンド関数。 */
  /*********************/
  // uciコマンド。
  void UCIShell::CommandUCI() {
    // IDを表示。
    std::cout << "id name " << ID_NAME << std::endl;
    std::cout << "id author " << ID_AUTHOR << std::endl;

    // TODO: 変更可能オプションの表示。

    std::cout << "uciok" << std::endl;
  }
  // isreadyコマンド。
  void UCIShell::CommandIsReady() {
    // TODO: パラメータの変更。

    std::cout << "readyok" << std::endl;
  }
  // quitコマンド。
  void UCIShell::CommandQuit() {
  }
  // positionコマンド。
  void UCIShell::CommandPosition(std::vector<std::string>& argv) {
    // コマンドをパース。
    int i = 0;
    int len = argv.size();

    // ポジションコマンドがあるかどうか。
    while (i < len) {
      if (argv[i] == "position") {
        i++;
        break;
      }
      i++;
    }
    if (i >= len) return;

    // 駒の配置をパース。
    while (i < len) {
      if (argv[i] == "startpos") {
        engine_ptr_->LoadFen(Fen(std::string
        ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")));
        i++;
        break;
      } else if (argv[i] == "fen") {
        i++;
        if (i >= len) return;
        // FENを合体。
        std::string fen_str = "";
        while (i < len) {
          if (argv[i] == "moves") {
            i--;
            break;
          }
          fen_str += argv[i] + " ";
          i++;
        }

        engine_ptr_->LoadFen(Fen(fen_str));
        i++;
        break;
      }
      i++;
    }
    if (i >= len) return;

    // 手サブコマンドをパース。
    while (i < len) {
      if (argv[i] == "moves") {
        i++;
        break;
      }
      i++;
    }
    if (i >= len) return;

    // 手をパース。
    Move move;
    while (i < len) {
      move.all_ = 0;

      // fromをパース。
      if ((argv[i][0] < 'a') || (argv[i][0] > 'h')) {
        i++;
        continue;
      }
      int fyle = argv[i][0] - 'a';
      move.from_ |= fyle;
      if ((argv[i][1] < '1') || (argv[i][1] > '8')) {
        i++;
        continue;
      }
      int rank = argv[i][1] - '1';
      move.from_ |= (rank << 3);

      // toをパース。
      if ((argv[i][2] < 'a') || (argv[i][2] > 'h')) {
        i++;
        continue;
      }
      fyle = argv[i][2] - 'a';
      move.to_ |= fyle;
      if ((argv[i][3] < '1') || (argv[i][3] > '8')) {
        i++;
        continue;
      }
      rank = argv[i][3] - '1';
      move.to_ |= (rank << 3);

      // 昇格をパース。
      if (argv[i].size() >= 5) {
        switch (argv[i][4]) {
          case 'n':
            move.promotion_ = KNIGHT;
            break;
          case 'b':
            move.promotion_ = BISHOP;
            break;
          case 'r':
            move.promotion_ = ROOK;
            break;
          case 'q':
            move.promotion_ = QUEEN;
            break;
          default:
            break;
        }
      }

      // 手を指す。
      engine_ptr_->PlayMove(move);
      i++;
    }
  }
}  // namespace Sayuri
