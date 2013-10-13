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
#include <thread>
#include <chrono>
#include <sstream>
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
  extern void Start();
  extern void Stop();
  extern int GetTime();
  void UCIShell::Test() {
    // positionコマンド。
    std::string command = "position startpos";
    std::cout << "Command: " << command << std::endl;

    std::vector<std::string> argv =
    MyLib::Split(command, " ", "");

    CommandPosition(argv);
    PrintPosition(engine_ptr_->position());

    // goコマンド。
    command = "go depth 12";
    argv = MyLib::Split(command, " ", "");
    Start();
    CommandGo(argv);
    // std::this_thread::sleep_for(Chrono::milliseconds(30000));
    // engine_ptr_->StopCalculation();
    thinking_thread_.join();
    Stop();
    std::cout << "Thinking Time: " << GetTime() << std::endl;
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
    for (std::size_t i = 0; i < pv_line.length(); i++) {
      if (pv_line.line()[i].has_checkmated()) break;

      std::cout << " " << TransMoveToString(pv_line.line()[i].move());
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
    std::cout << "info currmove " << TransMoveToString(move);

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

  // 思考スレッド。
  void UCIShell::ThreadThinking(UCIShell& shell) {
    // 思考準備。
    std::unique_ptr<TranspositionTable> table_ptr(new TranspositionTable
    (shell.table_size_));

    // 思考開始。
    PVLine pv_line = shell.engine_ptr_->Calculate
    (*(table_ptr.get()), shell.moves_to_search_ptr_.get());

    // 最善手を表示。
    std::cout << "bestmove ";
    if ((pv_line.length() > 0)
    && (!(pv_line.line()[0].has_checkmated()))) {
      std::string move_str = TransMoveToString(pv_line.line()[0].move());
      std::cout << move_str;

      // 2手目があるならponderで表示。
      if ((pv_line.length() >= 2)
      && (!(pv_line.line()[1].has_checkmated()))) {
        move_str = TransMoveToString(pv_line.line()[1].move());
        std::cout << " ponder " << move_str;
      }
    }
    std::cout << std::endl;
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
    // TODO: プログラムの終了。
  }
  // positionコマンド。
  void UCIShell::CommandPosition(std::vector<std::string>& argv) {
    // コマンドをパース。
    std::vector<std::string>::iterator itr = argv.begin();
    std::vector<std::string>::iterator end_itr = argv.end();

    // ポジションコマンドがあるかどうか。
    while (itr < end_itr) {
      if (*itr == "position") {
        itr++;
        break;
      }
      itr++;
    }

    // 駒の配置をパース。
    while (itr < end_itr) {
      if (*itr == "startpos") {
        engine_ptr_->LoadFen(Fen(std::string
        ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")));
        itr++;
        break;
      } else if (*itr == "fen") {
        itr++;
        if (itr >= end_itr) return;
        // FENを合体。
        std::string fen_str = "";
        while (itr < end_itr) {
          if (*itr == "moves") {
            itr--;
            break;
          }
          fen_str += *itr + " ";
          itr++;
        }

        engine_ptr_->LoadFen(Fen(fen_str));
        itr++;
        break;
      }
      itr++;
    }

    // 手サブコマンドをパース。
    while (itr < end_itr) {
      if (*itr == "moves") {
        itr++;
        break;
      }
      itr++;
    }

    // 手をパース。
    Move move;
    while (itr < end_itr) {
      move = TransStringToMove(*itr);
      // 手を指す。
      if (move.all_) {
        engine_ptr_->PlayMove(move);
      }
      itr++;
    }
  }

  // goコマンド。
  void UCIShell::CommandGo(std::vector<std::string>& argv) {
    // スレッドが実行中なら中止する。
    if (thinking_thread_.joinable()) {
      engine_ptr_->StopCalculation();
      thinking_thread_.join();
    }

    // 準備。
    std::vector<std::string>::iterator itr = argv.begin();
    std::vector<std::string>::iterator end_itr = argv.end();
    int max_depth = MAX_PLYS;
    std::size_t max_nodes = MAX_NODES;
    Chrono::milliseconds thinking_time(60000);
    bool infinite_thinking = false;

    // goトークンまで流す。
    while (itr < end_itr) {
      if (*itr == "go") {
        itr++;
        break;
      }
      itr++;
    }

    // サブコマンドをパース。
    Move move;
    Chrono::milliseconds time_control(0);
    while (itr < end_itr) {
      if (*itr == "searchmoves") {
        // searchmovesコマンド。
        itr++;
        // 手をベクトルに格納していく。
        while (itr < end_itr) {
          move = TransStringToMove(*itr);
          if (move.all_) {
            moves_to_search_ptr_->push_back(move);
            itr++;
          } else {
            itr--;
            break;
          }
        }
      } else if ((*itr == "ponder") || (*itr == "infinite")) {
        // ポンダリングモードで探索する。
        infinite_thinking = true;
      } else if ((*itr == "wtime") && (engine_ptr_->to_move() == WHITE)) {
        // 白の持ち時間。エンジンが白番の場合。
        itr++;
        try {
          time_control = Chrono::milliseconds(std::stoull(*itr));
          if (time_control.count() > 180000) {
            thinking_time = Chrono::milliseconds(120000);
          } else {
            thinking_time = time_control / 2;
          }
        } catch (...) {
          itr--;
        }
      } else if ((*itr == "btime") && (engine_ptr_->to_move() == BLACK)) {
        // 黒の持ち時間。エンジンが黒番の場合。
        itr++;
        try {
          time_control = Chrono::milliseconds(std::stoull(*itr));
          if (time_control.count() > 180000) {
            thinking_time = Chrono::milliseconds(120000);
          } else {
            thinking_time = time_control / 2;
          }
        } catch (...) {
          itr--;
        }
      } else if (*itr == "depth") {
        // 深さの指定。
        itr++;
        try {
          max_depth = std::stoi(*itr);
        } catch (...) {
          itr--;
        }
      } else if (*itr == "nodes") {
        // 探索するノード数の指定。
        itr++;
        try {
          max_nodes = std::stoull(*itr);
        } catch (...) {
          itr--;
        }
      } else if (*itr == "movetime") {
        // 探索時間の指定。
        itr++;
        try {
          thinking_time = Chrono::milliseconds(std::stoull(*itr));
        } catch (...) {
          itr--;
        }
      } else if (*itr == "mate") {
        // メイトを探す。
        itr++;
        try {
          max_depth = (std::stoi(*itr) * 2) - 1;
        } catch (...) {
          itr--;
        }
      }
      itr++;
    }

    // 別スレッドで思考開始。
    engine_ptr_->SetStopper(max_depth, max_nodes, thinking_time,
    infinite_thinking);
    thinking_thread_ = std::thread(ThreadThinking, std::ref(*this));
  }

  /**************/
  /* 便利関数。 */
  /**************/
  // 手を文字列に変換。
  std::string UCIShell::TransMoveToString(Move move) {
    // 文字列ストリーム。
    std::ostringstream oss;

    // ストリームに流しこむ。
    oss << static_cast<char>(Util::GetFyle(move.from_) + 'a');
    oss << static_cast<char>(Util::GetRank(move.from_) + '1');
    oss << static_cast<char>(Util::GetFyle(move.to_) + 'a');
    oss << static_cast<char>(Util::GetRank(move.to_) + '1');
    switch (move.promotion_) {
      case KNIGHT:
        oss << 'n';
        break;
      case BISHOP:
        oss << 'b';
        break;
      case ROOK:
        oss << 'r';
        break;
      case QUEEN:
        oss << 'q';
        break;
      default:
        break;
    }

    return oss.str();
  }

  // 文字列を手に変換。
  Move UCIShell::TransStringToMove(std::string move_str) {
    Move move, null_move;

    if (move_str.size() < 4) return null_move;

    // fromをパース。
    if ((move_str[0] < 'a') || (move_str[0] > 'h')) {
      return null_move;
    }
    int fyle = move_str[0] - 'a';
    move.from_ |= fyle;
    if ((move_str[1] < '1') || (move_str[1] > '8')) {
      return null_move;
    }
    int rank = move_str[1] - '1';
    move.from_ |= (rank << 3);

    // toをパース。
    if ((move_str[2] < 'a') || (move_str[2] > 'h')) {
      return null_move;
    }
    fyle = move_str[2] - 'a';
    move.to_ |= fyle;
    if ((move_str[3] < '1') || (move_str[3] > '8')) {
      return null_move;
    }
    rank = move_str[3] - '1';
    move.to_ |= (rank << 3);

    // 昇格をパース。
    if (move_str.size() >= 5) {
      switch (move_str[4]) {
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

    return move;
  }
}  // namespace Sayuri
